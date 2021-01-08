// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_OPERATION_PARAM_H
#define MOCK_KEY_VALUE_STORE_OPERATION_PARAM_H

namespace mockdb {
    template <typename K, typename V>
    class operation_param {
    public:
        virtual K get_key() const {
            return this->key;
        }

        virtual ~operation_param() {
            // Pass
        }

    protected:
        K key;
    };

    template <typename K, typename V>
    class GET_param : public operation_param<K,V> {
    public:
        GET_param(const K &key) {
            this->key = key;
        }
    };

    template <typename K, typename V>
    class PUT_param : public operation_param<K, V> {
    public:
        PUT_param(const K &key, const V &value) {
            this->key = key;
            this->value = value;
        }
    private:
        V value;
    };

    template <typename K, typename V>
    class REMOVE_param : public operation_param<K, V> {
    public:
        REMOVE_param(const K &key) {
            this->key = key;
        }
    };
}
#endif //MOCK_KEY_VALUE_STORE_OPERATION_PARAM_H
