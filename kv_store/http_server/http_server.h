// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Simple API for HTTP server - compatible with DAPR

#ifndef MOCK_KEY_VALUE_STORE_HTTP_SERVER_H
#define MOCK_KEY_VALUE_STORE_HTTP_SERVER_H

#include "../include/kv_store.h"
#include "../include/read_response_selector.h"

#include <cpprest/http_listener.h>

namespace mockdb {
    template <typename K, typename V>
    class http_server {
    public:
        http_server(utility::string_t url);
        ~http_server();

        pplx::task<void> open() {
            return m_listener.open();
        }
        pplx::task<void> close() {
            return m_listener.close();
        }

        void handle_get(web::http::http_request message);
        void handle_post(web::http::http_request message);
        void handle_delete(web::http::http_request message);

    private:
        web::http::experimental::listener::http_listener m_listener;
        kv_store<K, V> *store;
        read_response_selector<K, V> *get_next_tx;
        std::mutex mtx;     // Required for concurrent PUT
                            // operations which might use ETag

        long get_session_id (web::http::http_headers headers);
    };
}

/*
 * Initialize server with url
 */
template <typename K, typename V>
mockdb::http_server<K, V>::http_server(utility::string_t url) : m_listener(url) {
    get_next_tx = new causal_read_response_selector<K, V>();
//    get_next_tx = new linearizable_read_response_selector<K, V>();
    store = new kv_store<K, V>(get_next_tx);
    get_next_tx->init_consistency_checker(store);
    m_listener.support(web::http::methods::GET, std::bind(&http_server::handle_get, this, std::placeholders::_1));
    m_listener.support(web::http::methods::POST, std::bind(&http_server::handle_post, this, std::placeholders::_1));
    m_listener.support(web::http::methods::DEL, std::bind(&http_server::handle_delete, this, std::placeholders::_1));
}

template <typename K, typename V>
mockdb::http_server<K, V>::~http_server() {
    delete this->store;
    delete this->get_next_tx;
}

/*
 * Finds session id in HTTP headers, if not present returns default session ID.
 */
template <typename K, typename V>
long mockdb::http_server<K, V>::get_session_id(web::http::http_headers headers) {
    auto it = headers.find("session-id");
    if (it != headers.end())
        return std::stol(it->second);
    return DEFAULT_SESSION;
}

/*
 * Handle get requests.
 * Required format: http://localhost:${port}/v1.0/state/${stateStoreName}/${key}
 */
template <typename K, typename V>
void mockdb::http_server<K, V>::handle_get(web::http::http_request message) {
#ifdef MOCKDB_DEBUG_LOG
    std::cout << "[MOCKDB::kvstore] GET Request received from "
                << message.remote_address() << std::endl;
    std::cout << message.to_string() << std::endl;
    for (auto header : message.headers()) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
#endif // MOCKDB_DEBUG_LOG

    long session_id = get_session_id(message.headers());
    auto paths = web::http::uri::split_path(web::http::uri::decode(message.relative_uri().path()));
    web::json::value response;

    if (paths.size() != 4) {
        response["error"] = web::json::value("Bad request");
        message.reply(web::http::status_codes::BadRequest, response);
        return;
    }

    std::pair<V, size_t> value_version;
    web::http::http_response http_response;

    try {
        value_version = store->get_with_version(paths[3], session_id);
        http_response.headers().add("Etag", value_version.second);
    } catch (key_not_found_exception &e) {
        message.reply(web::http::status_codes::NotFound);
        return;
    } catch (consistency_exception &e) {
        response["error"] = web::json::value("No consistent response possible");
        message.reply(web::http::status_codes::OK, response);
        return;
    }
    http_response.set_status_code(web::http::status_codes::OK);
    http_response.set_body(web::json::value(value_version.first));
    message.reply(http_response);
}

/*
 * Handle post requests.
 * Expects key-value pair(s) in JSON body type.
 */
template <typename K, typename V>
void mockdb::http_server<K, V>::handle_post(web::http::http_request message) {
    std::lock_guard<std::mutex> lck (mtx);
#ifdef MOCKDB_DEBUG_LOG
    std::cout << "[MOCKDB::kvstore] PUT Request received from "
                << message.remote_address() << std::endl;
    std::cout << message.to_string() << std::endl;
    for (auto header : message.headers()) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
#endif // MOCKDB_DEBUG_LOG

    long session_id = get_session_id(message.headers());
    web::json::value payload = message.extract_json().get();
    K key = payload.at(U("key")).as_string();
    V value = payload.at(U("value"));
    web::json::value response;

    if (payload.has_field(U("etag"))) {
        std::string etag = payload.at(U("etag")).as_string();
        // Check if etag matches the latest version
        size_t version = 0;
        try {
            version = store->get_with_version(key, session_id).second;
        } catch (std::exception &e) {
            // PASS
        }

        if (std::to_string(version) != etag) {
#ifdef MOCKDB_DEBUG_LOG
            std::cout << "[MOCKDB::kvstore] [error] ETag mismatch" << std::endl;
#endif // MOCKDB_DEBUG_LOG
            response["success"] = web::json::value("false");
            response["description"] = web::json::value("ETag mismatch");
            message.reply(web::http::status_codes::BadRequest, response);
            return;
        }
    }

    store->put(key, value, session_id);
    response["success"] = web::json::value("true");
    message.reply(web::http::status_codes::OK, response);
}

/*
 *
 */
template <typename K, typename V>
void mockdb::http_server<K, V>::handle_delete(web::http::http_request message) {
    // TODO
}

#endif //MOCK_KEY_VALUE_STORE_HTTP_SERVER_H
