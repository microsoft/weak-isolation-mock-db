// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_OPERATION_H
#define MOCK_KEY_VALUE_STORE_OPERATION_H

#include "operation_param.h"
#include "operation_response.h"

namespace mockdb {
    template <typename K, typename V>
    class operation {
    public:
        operation(operation_param<K, V>* params) {
            this->params = params;
            this->response = nullptr;
        }
        virtual ~operation() {
            delete this->params;
            delete this->response;
        }

        virtual const operation_param<K, V>* get_params() const {
            return this->params;
        }
        virtual const operation_response<K, V>* get_response() const {
            return this->response;
        }
        virtual void set_response(operation_response<K, V> *response) {
            this->response = response;
        }
    protected:
        operation_param<K, V> *params;
        operation_response<K, V> *response;
    };

    template <typename K, typename V>
    class GET_operation : public operation<K, V> {
    public:
        GET_operation(GET_param<K, V>* params) : operation<K,V> (params) {

        }

        virtual const GET_param<K, V>* get_params() const {
            return dynamic_cast<const GET_param<K, V>*>(this->params);
        }
        virtual const GET_response<K, V>* get_response() const {
            return dynamic_cast<const GET_response<K, V>*>(this->response);
        }
        virtual void set_response(GET_response<K, V> *response) {
            this->response = response;
        }
    };

    template <typename K, typename V>
    class PUT_operation : public operation<K, V> {
    public:
        PUT_operation(PUT_param<K, V>* params) : operation<K,V> (params) {

        }

        virtual const PUT_param<K, V>* get_params() const {
            return dynamic_cast<const PUT_param<K, V>*>(this->params);
        }
        virtual const PUT_response<K, V>* get_response() const {
            return dynamic_cast<const PUT_response<K, V>*>(this->response);
        }
        virtual void set_response(PUT_response<K, V> *response) {
            this->response = response;
        }
    };

    template <typename K, typename V>
    class REMOVE_operation : public operation<K, V> {
    public:
        REMOVE_operation(REMOVE_param<K, V>* params) : operation<K,V> (params) {

        }

        virtual const REMOVE_param<K, V>* get_params() const {
            return dynamic_cast<const REMOVE_param<K, V>*>(this->params);
        }
        virtual const REMOVE_response<K, V>* get_response() const {
            return dynamic_cast<const REMOVE_response<K, V>*>(this->response);
        }
        virtual void set_response(REMOVE_response<K, V> *response) {
            this->response = response;
        }
    };
}
#endif //MOCK_KEY_VALUE_STORE_OPERATION_H
