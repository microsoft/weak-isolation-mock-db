// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_CONSISTENCY_CHECKER_H
#define MOCK_KEY_VALUE_STORE_CONSISTENCY_CHECKER_H


#include "kv_store.h"

#include <list>
#include <unordered_set>

namespace mockdb {
    /*
     * Given a data store, check if a new transaction is valid under the particular
     * consistency level.
     */
    template <typename K, typename V>
    class consistency_checker {
    public:
        consistency_checker(const kv_store<K, V> *store) {
            this->store = store;
        }

        virtual bool is_consistent(const transaction<K, V> *new_tx) = 0;
    protected:
        const kv_store<K, V> *store;
    };

    template <typename K, typename V>
    class causal_consistency_checker : public consistency_checker<K, V>{
    public:
        causal_consistency_checker(const kv_store<K, V> *store) : consistency_checker<K, V>(store){
        }

        bool is_consistent(const transaction<K, V> *new_tx) {
            // Find tx from which new_tx reads
            const GET_operation<K, V> *op = dynamic_cast<const GET_operation<K, V>*>(new_tx->get_operation());
            long tx_id = op->get_response()->get_written_by_tx_id();
            K key = op->get_response()->get_key();
            long session_id = new_tx->get_session_id();

            // Find (wr U so)+
            // Since only one operation is allowed per transaction, we just need to find
            // set of transactions which write the given key and are reachable from so
            std::unordered_set<long> dependent_txs;

            for (auto tx : this->store->get_session_history(session_id)) {
                const PUT_operation<K, V> *PUT_op =  dynamic_cast<const PUT_operation<K, V>*>(tx->get_operation());
                if (PUT_op && PUT_op->get_params()->get_key() == key) {
                    dependent_txs.insert(tx->get_tx_id());
                }
                else {
                    const GET_operation<K, V> *GET_op = dynamic_cast<const GET_operation<K, V>*>(tx->get_operation());
                    if (GET_op && GET_op->get_params()->get_key() == key) {
                        dependent_txs.insert(GET_op->get_response()->get_written_by_tx_id());
                    }
                }
            }
            if (dependent_txs.size() == 0)
                return true;

            // Check if all dependent_txs are committed before tx_id
            int committed_count = 0;
            for (auto tx : this->store->get_history()) {
                if (dependent_txs.find(tx->get_tx_id()) != dependent_txs.end())
                    committed_count++;
                if (tx->get_tx_id() == tx_id) {
                    if (committed_count == dependent_txs.size())
                        return true;
                    return false;
                }
            }
            return false;
        }
    };

}
#endif //MOCK_KEY_VALUE_STORE_CONSISTENCY_CHECKER_H
