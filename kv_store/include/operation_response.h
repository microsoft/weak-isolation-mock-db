// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_OPERATION_RESPONSE_H
#define MOCK_KEY_VALUE_STORE_OPERATION_RESPONSE_H

namespace mockdb {
    template <typename K, typename V>
    class operation_response {
    public:
        virtual bool is_successful() {
            return success;
        }
    protected:
        bool success;
    };

    template <typename K, typename V>
    class GET_response : public operation_response<K, V> {
    public:
        GET_response(const K &k, const V &v) : key(k), value(v){
            this->success = true;
        }

        const K get_key() const {
            return key;
        }

        const V get_value() const {
            return value;
        }
        long get_written_by_tx_id() const {
            return written_by_tx_id;
        }
        void set_written_by_tx_id(long value) {
            written_by_tx_id = value;
        }
    private:
        const K key;
        const V value;
        long written_by_tx_id;
    };

    template <typename K, typename V>
    class PUT_response : public operation_response<K, V> {
    public:
        PUT_response(bool success) {
            this->success = success;
        }
    };

    template <typename K, typename V>
    class REMOVE_response : public operation_response<K, V> {

    };
}

#endif //MOCK_KEY_VALUE_STORE_OPERATION_RESPONSE_H
