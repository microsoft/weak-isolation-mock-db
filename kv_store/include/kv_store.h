// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Key-value store API


#ifndef MOCK_KEY_VALUE_STORE_KV_STORE_H
#define MOCK_KEY_VALUE_STORE_KV_STORE_H

//#define MOCKDB_DEBUG_LOG
#define DEFAULT_SESSION 1

#include "transaction.h"
#include "key_not_found_exception.h"
#include "query_parse_exception.h"
#include "consistency_exception.h"

#include <list>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <sstream>

namespace mockdb {
    // Forward declaration of class
    template<typename K, typename V>
    class read_response_selector;

    template <typename K, typename V>
    class kv_store {
    public:
        kv_store(read_response_selector<K, V> *read_selector);

        // KV interface
        V get(const K &key, long session_id = DEFAULT_SESSION);
        std::pair<V, size_t> get_with_version(const K &key, long session_id = DEFAULT_SESSION);
        int put(const K &key, const V &value, long session_id = DEFAULT_SESSION);
        V remove(const K &key, long session_id = DEFAULT_SESSION);

        size_t get_size() const;
        virtual ~kv_store();

        const read_response_selector<K, V> *get_gen_next_tx() const;
        void set_gen_next_tx(read_response_selector<K, V> *gen_next_tx);

        const std::list<transaction<K, V>*> get_session_history(long session_id) const;
        const std::list<transaction<K, V>*> get_history() const;

    private:
        mockdb::transaction<K, V> *_get(const K &key, long session_id = DEFAULT_SESSION);
        void commit_tx(transaction<K, V> *tx, long session_id);

        std::unordered_map<K, std::list<std::pair<V, long>>> kv_map;
        std::list<transaction<K, V>*> history;
        std::unordered_map<long, std::list<transaction<K, V>*>> session_order;
        std::mutex mtx;
        read_response_selector<K, V> *read_selector;
    };

}

template <typename K, typename V>
mockdb::kv_store<K, V>::kv_store(read_response_selector<K, V> *get_next_tx) {
    this->read_selector = get_next_tx;
}

// Destructor
template <typename K, typename V>
mockdb::kv_store<K, V>::~kv_store() {
    for (auto tx : this->history) {
        delete tx;
    }
}

/*
 * GET operation: returns value corresponding to the given key.
 * May throw key_not_found_exception.
 */
template <typename K, typename V>
V mockdb::kv_store<K, V>::get(const K &key, long session_id) {
    const GET_response<K, V> *op_response = dynamic_cast<const GET_response<K, V>*>(_get(key, session_id)->get_operation()->get_response());
    return op_response->get_value();
}

/*
 * GET operation: returns value along with the version number.
 * May throw key_not_found_exception.
 */
template <typename K, typename V>
std::pair<V, size_t> mockdb::kv_store<K, V>::get_with_version(const K &key, long session_id) {
    const GET_response<K, V> *op_response = dynamic_cast<const GET_response<K, V>*>(_get(key, session_id)->get_operation()->get_response());
    return {op_response->get_value(), op_response->get_version_number()};
}


/*
 * GET operation.
 * May throw key_not_found_exception.
 */
template <typename K, typename V>
mockdb::transaction<K, V> *mockdb::kv_store<K, V>::_get(const K &key, long session_id) {
    // Create GET operation and transaction
    GET_param<K, V> *params = new GET_param<K, V>(key);
    GET_operation<K, V> *op = new GET_operation<K, V>(params);
    transaction<K, V> *tx = new transaction<K, V>(op);
    tx->set_session_id(session_id);

    // Acquire the lock and enter critical section.
    this->mtx.lock();
    tx->start_transaction();

    // Throw exception if key doesn't exist
    if (this->kv_map.find(key) == this->kv_map.end()) {
#ifdef MOCKDB_DEBUG_LOG
        std::cout << "[MOCKDB::kvstore] [ERROR::KEY_NOT_FOUND] TXN " << tx->get_tx_id()
                  << " GET " << key << " NOTFOUND " << session_id << std::endl;
#endif // MOCKDB_DEBUG_LOG
        this->mtx.unlock();
        delete tx;
        std::stringstream ss;
        ss << key;
        throw key_not_found_exception(ss.str());
    }

    // List down candidate responses using all possible versions present in the store
    // corresponding to the given key
    std::vector<GET_response<K, V>*> candidate_responses;
    for (auto candidate_value : this->kv_map[key]) {
        GET_response<K, V> *op_response = new GET_response<K, V>(key, candidate_value.first);
        op_response->set_written_by_tx_id(candidate_value.second);
        candidate_responses.push_back(op_response);
    }

    GET_response<K, V> *op_response;
    try {
        // Choose one transaction response among the candidates by some strategy
        op_response = read_selector->select_read_response(tx, op, candidate_responses);
    } catch (consistency_exception &e) {
        // No consistent response possible
#ifdef MOCKDB_DEBUG_LOG
        std::cout << "[MOCKDB::kvstore] [ERROR::INCONSISTENT_STATE] TXN " << tx->get_tx_id()
                  << " GET " << key << " INCONSISTENT " << session_id << std::endl;
#endif // MOCKDB_DEBUG_LOG
        this->mtx.unlock();
        delete tx;
        for (auto c : candidate_responses)
            delete c;
        std::stringstream ss;
        ss << "GET(" << key << ")";
        throw consistency_exception(ss.str(), tx->get_tx_id());
    }

    op->set_response(op_response);
    tx->end_transaction();
    this->commit_tx(tx, session_id);

#ifdef MOCKDB_DEBUG_LOG
    std::cout << "[MOCKDB::kvstore] TXN " << tx->get_tx_id() << " GET " << key
              << " session " << session_id << std::endl;
#endif // MOCKDB_DEBUG_LOG

    // Free allocated memory and find out version number
    size_t version_number = 1;
    for (auto candidate : candidate_responses) {
        if (candidate != op_response) {
            version_number++;
            delete candidate;
        }
        else {
            op_response->set_version_number(version_number);
        }
    }

    // Done with critical section, release the lock.
    this->mtx.unlock();

    return tx;
}

/*
 * PUT operation.
 */
template <typename K, typename V>
int mockdb::kv_store<K, V>::put(const K &key, const V &value, long session_id) {
    // Create PUT operation and transaction
    PUT_param<K, V> *params = new PUT_param<K, V>(key, value);
    PUT_operation<K, V> *op = new PUT_operation<K, V>(params);
    transaction<K, V> *tx = new transaction<K, V>(op);
    tx->set_session_id(session_id);

    // Acquire the lock and enter critical section.
    this->mtx.lock();
    tx->start_transaction();

    if (this->kv_map.find(key) == this->kv_map.end()) {
        std::list<std::pair<V, long>> l({{value, tx->get_tx_id()}});
        this->kv_map[key] = l;
    }
    else
        this->kv_map[key].push_back({value, tx->get_tx_id()});

    tx->end_transaction();

    // Record response of transaction
    PUT_response<K, V> *op_response = new PUT_response<K, V>(true);
    op->set_response(op_response);

    this->commit_tx(tx, session_id);

#ifdef MOCKDB_DEBUG_LOG
    std::cout << "[MOCKDB::kvstore] TXN " << tx->get_tx_id() << " PUT " << key
         << " session " << session_id << std::endl;
#endif // MOCKDB_DEBUG_LOG

    // Done with critical section, release the lock.
    this->mtx.unlock();
    return 1;
}

/*
 * REMOVE operation.
 * May throw key_not_found_exception.
 */
template <typename K, typename V>
V mockdb::kv_store<K, V>::remove(const K &key, long session_id) {
    // TODO
}

template <typename K, typename V>
void mockdb::kv_store<K, V>::commit_tx(transaction<K, V> *tx, long session_id) {
    // Insert transaction in history
    this->history.push_back(tx);
    if (this->session_order.find(session_id) == this->session_order.end())
        this->session_order[session_id] = std::list<transaction<K, V>*>({tx});
    else
        this->session_order[session_id].push_back(tx);
}

template<typename K, typename V>
const mockdb::read_response_selector<K, V> *mockdb::kv_store<K, V>::get_gen_next_tx() const {
    return read_selector;
}

// Get number of keys in the store
template<typename K, typename V>
size_t mockdb::kv_store<K, V>::get_size() const {
    return this->kv_map.size();
}

template<typename K, typename V>
void mockdb::kv_store<K, V>::set_gen_next_tx(mockdb::read_response_selector<K, V> *get_next_tx) {
    this->read_selector = get_next_tx;
}

template<typename K, typename V>
const std::list<mockdb::transaction<K, V>*> mockdb::kv_store<K, V>::get_history() const {
    return this->history;
}

template<typename K, typename V>
const std::list<mockdb::transaction<K, V>*> mockdb::kv_store<K, V>::get_session_history(long session_id) const {
    if (this->session_order.find(session_id) == this->session_order.end())
        return std::list<transaction<K, V>*>();
    return this->session_order.at(session_id);
}


#endif //MOCK_KEY_VALUE_STORE_KV_STORE_H
