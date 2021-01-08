// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Selects one read response among multiple valid responses using consistency specification.

#ifndef MOCK_KEY_VALUE_STORE_READ_RESPONSE_SELECTOR_H
#define MOCK_KEY_VALUE_STORE_READ_RESPONSE_SELECTOR_H


#include "kv_store.h"
#include "consistency_checker.h"

#include <list>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <numeric>

namespace mockdb {
    template<typename K, typename V>
    class read_response_selector {
    public:
        read_response_selector() {
            this->store = nullptr;
        }

        virtual ~read_response_selector() {

        }

        virtual void init_consistency_checker(const kv_store <K, V> *store) = 0;

        // By default it picks one transaction response at random
        virtual GET_response<K, V> *select_read_response(transaction<K, V> *,
                                                         GET_operation<K, V> *,
                                                         std::vector<GET_response<K, V> *> candidates) {
            int idx = std::rand() % candidates.size();
            return candidates[idx];
        }

    protected:
        const kv_store<K, V> *store;
    };

    template<typename K, typename V>
    class causal_read_response_selector : public read_response_selector<K, V> {
    public:
        causal_read_response_selector() {
            this->checker = nullptr;
        }

        ~causal_read_response_selector() {
            delete this->checker;
        }

        void init_consistency_checker(const kv_store <K, V> *store) {
            this->store = store;
            this->checker = new causal_consistency_checker<K, V>(this->store);
        }

        GET_response<K, V> *select_read_response(transaction<K, V> *tx,
                                                 GET_operation<K, V> *op,
                                                 std::vector<GET_response<K, V> *> candidates) {

            int limit = candidates.size();

            std::random_device rd; // obtain a random number from hardware
            std::mt19937 gen(rd()); // seed the generator

            while (limit > 0) {
                int index = -1;
                // Choose one candidate randomly
                std::uniform_int_distribution<> dist(0, limit - 1);
                index = dist(gen);

                // Swap with the last element to not pick the same element again
                swap(candidates[index], candidates[limit - 1]);
                GET_response<K, V> *candidate = candidates[limit - 1];
                limit--;

                op->set_response(candidate);
                if (checker->is_consistent(tx)) {
                    return candidate;
                }
            }

            // No consistent transaction response possible
            op->set_response(nullptr);
            throw consistency_exception("GET", tx->get_tx_id());
        }

    private:
        consistency_checker<K, V> *checker;
    };

    template<typename K, typename V>
    class linearizable_read_response_selector : public read_response_selector<K, V> {
    public:
        linearizable_read_response_selector() {
        }

        ~linearizable_read_response_selector() {
        }

        void init_consistency_checker(const kv_store<K, V> *store) {
            this->store = store;
        }

        GET_response<K, V> *select_read_response(transaction<K, V> *tx,
                                                 GET_operation<K, V> *op,
                                                 std::vector<GET_response<K, V> *> candidates) {
            if (candidates.empty()) {
                // No consistent transaction response possible
                op->set_response(nullptr);
                throw consistency_exception("GET", tx->get_tx_id());
            }
            op->set_response(candidates.back());
            return candidates.back();
        }
    };

    // k-causal : at most k times weaker than linearizable
    template<typename K, typename V>
    class k_causal_read_response_selector : public read_response_selector<K, V> {
    public:
        k_causal_read_response_selector(int k, int total_read_count) {
            this->k = k;
            this->total_read_count = total_read_count;
            this->causal_selector = new causal_read_response_selector<K, V>();
            this->linearizable_selector = new linearizable_read_response_selector<K, V>();
        }

        ~k_causal_read_response_selector() {
            delete this->causal_selector;
            delete this->linearizable_selector;
        }

        void init_consistency_checker(const kv_store<K, V> *store) {
            this->causal_selector->init_consistency_checker(store);
            this->linearizable_selector->init_consistency_checker(store);
            // Pick k causal reads
            pick_k_read_ids();
        }

        GET_response<K, V> *select_read_response(transaction<K, V> *tx,
                                                 GET_operation<K, V> *op,
                                                 std::vector<GET_response<K, V> *> candidates) {

            this->read_count++;
            if (this->k_read_ids.find(this->read_count) != this->k_read_ids.end()) {
                return this->causal_selector->select_read_response(tx, op, candidates);
            }
            return this->linearizable_selector->select_read_response(tx, op, candidates);
        }

    private:
        int k, total_read_count;
        int read_count = 0;
        std::unordered_set<int> k_read_ids;
        causal_read_response_selector<K, V> *causal_selector;
        linearizable_read_response_selector<K, V> *linearizable_selector;

        void pick_k_read_ids() {
            std::vector<int> read_ids(this->total_read_count);
            std::iota(read_ids.begin(), read_ids.end(), 1);
            std::shuffle(read_ids.begin(), read_ids.end(), std::mt19937{std::random_device{}()});
            k_read_ids.insert(read_ids.begin(), read_ids.begin() + this->k);
        }
    };
}
#endif //MOCK_KEY_VALUE_STORE_READ_RESPONSE_SELECTOR_H
