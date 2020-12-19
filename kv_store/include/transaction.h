// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_TRANSACTION_H
#define MOCK_KEY_VALUE_STORE_TRANSACTION_H

#include "operation.h"

#include <atomic>
#include <iostream>

namespace mockdb {
    template <typename K, typename V>
    class transaction {
    public:
        transaction(operation<K, V> *op) {
            this->tx_id = this->generate_tx_id();
            this->op = op;
#ifdef MOCKDB_DEBUG_LOG
            std::cout << "[MOCKDB::kvstore] New transaction created ID: "
                        << this->tx_id << std::endl;
#endif // MOCKDB_DEBUG_LOG
        }

        virtual ~transaction() {
            delete this->op;
        }

        virtual void start_transaction() {}
        virtual void end_transaction() {}

        // Getters and setters
        long get_tx_id() const;
        const operation<K, V> *get_operation() const;
        void set_operation(operation<K, V> *op);
        long get_session_id() const;
        void set_session_id(long session_id);

    protected:
        static std::atomic_long tx_count;
        long tx_id, session_id;
        operation<K, V> *op;

    private:
        long generate_tx_id() {
            return ++(this->tx_count);
        }
    };
}

// Initialize static variable
template <typename K, typename V>
std::atomic_long mockdb::transaction<K, V>::tx_count(0);

template <typename K, typename V>
long mockdb::transaction<K, V>::get_tx_id() const {
    return tx_id;
}

template <typename K, typename V>
long mockdb::transaction<K, V>::get_session_id() const {
    return session_id;
}

template <typename K, typename V>
void mockdb::transaction<K, V>::set_session_id(long session_id) {
    this->session_id = session_id;
}

template <typename K, typename V>
const mockdb::operation<K, V> *mockdb::transaction<K, V>::get_operation() const {
    return this->op;
}

template <typename K, typename V>
void mockdb::transaction<K, V>::set_operation(mockdb::operation<K, V> *operation) {
    this->op = op;
}

#endif //MOCK_KEY_VALUE_STORE_TRANSACTION_H
