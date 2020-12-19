// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_SHOPPING_CART_H
#define MOCK_KEY_VALUE_STORE_SHOPPING_CART_H

#include "item.h"
#include "user.h"
#include "../utils.h"
#include "../../kv_store/include/kv_store.h"

#include <vector>
#include <cpprest/json.h>
#include <thread>

/*
 * Format in which data is stored in the kv-store
    key    : value
    string : json

    "item:id:name" : {
                        xxx
                     }

    "item:id:price" : {
                        xxx
                     }

    "user:id:name" : {
                        xxx
                     }


    "cart:user_id:item_id:quantity" : {
                                        5
                                      }

    "cart:user_id" : {
                        "items" :
                                [
                                    "item_id",
                                    "item_id",
                                    ....
                                 ],
                        "count" :	3
                     }
 */
class shopping_cart {
public:
    shopping_cart(user u, mockdb::kv_store<std::string, web::json::value> *store,
                  consistency consistency_level);
    void add_item(item i, long session_id = 1);
    void add_item(item i, int quantity, long session_id = 1);
    void change_quantity(item i, int quantity, long session_id = 1);
    void remove_item(item i, long session_id = 1);
    item get_item(int item_id, long session_id = 1);
    int get_quantity(item i, long session_id = 1);
    std::vector<std::pair<item, int>> get_cart_list(long session_id = 1);
    double get_bill(long session_id = 1);

    void tx_start();
    void tx_end();


private:
    int user_id;
    mockdb::kv_store<std::string, web::json::value> *store;
    std::mutex mtx;
    consistency consistency_level;

    void _add_item(int id, long session_id = 1);
    void _add_quantity(int id, int quantity, long session_id = 1);
    void _change_quantity(int id, int quantity, long session_id = 1);
    int _get_quantity(int id, long session_id = 1);
};

shopping_cart::shopping_cart(user u, mockdb::kv_store<std::string, web::json::value> *store,
                             consistency consistency_level) {
    this->user_id = u.id;
    this->store = store;
    this->consistency_level = consistency_level;
}

void shopping_cart::tx_start() {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 4));
    mtx.lock();
}

void shopping_cart::tx_end() {
    mtx.unlock();
}

void shopping_cart::add_item(item i, long session_id) {
    _add_item(i.id, session_id);
    _add_quantity(i.id, 1, session_id);
}

void shopping_cart::add_item(item i, int quantity, long session_id) {
    _add_item(i.id, session_id);
    _add_quantity(i.id, quantity, session_id);
}

void shopping_cart::change_quantity(item i, int quantity, long session_id) {
    int old_quantity = _get_quantity(i.id, session_id);

    // update quantity if item has not been deleted
    if (old_quantity != 0)
        _change_quantity(i.id, quantity, session_id);
}

void shopping_cart::remove_item(item i, long session_id) {
    std::string user = std::to_string(user_id);

    web::json::value cart;
    try {
        cart = this->store->get("cart:" + user, session_id);
    } catch (std::exception &e) {
        return;
    }

    web::json::array items = cart["items"].as_array();
    std::vector<web::json::value> new_items;
    for (web::json::array::iterator it = items.begin(); it != items.end(); it++) {
        if (*it != web::json::value(i.id)) {
            new_items.push_back(*it);
        }
        else {  // Mark quantity as 0
            _change_quantity(i.id, 0, session_id);
        }
    }
    cart["items"] = web::json::value::array(new_items);
    cart["count"] = web::json::value(new_items.size());
    this->store->put("cart:" + user, cart, session_id);
}

int shopping_cart::get_quantity(item i, long session_id) {
    return _get_quantity(i.id, session_id);
}

item shopping_cart::get_item(int item_id, long session_id) {
    std::string name = this->store->get("item:" + std::to_string(item_id) + ":name", session_id).as_string();
    double price = this->store->get("item:" + std::to_string(item_id) + ":price", session_id).as_double();
    item i(name, item_id, price);
    return i;
}

std::vector<std::pair<item, int>> shopping_cart::get_cart_list(long session_id) {
    std::vector<std::pair<item, int>> cart_list;
    web::json::value cart;
    try {
        cart = this->store->get("cart:" + std::to_string(this->user_id), session_id);
    } catch (std::exception &e) {
        return cart_list;
    }

    web::json::array items = cart["items"].as_array();
    for (auto &i : items) {
        int quantity = _get_quantity(i.as_integer(), session_id);
        std::string name = this->store->get("item:" + std::to_string(i.as_integer()) + ":name", session_id)
                                        .as_string();
        double price = this->store->get("item:" + std::to_string(i.as_integer()) + ":price", session_id)
                                    .as_double();
        item it(name, i.as_integer(), price);
        cart_list.push_back({it, quantity});
    }

    std::stringstream ss;
    for (auto e : cart_list) {
        ss << e.first.name << " " << e.second << " ";
    }

#ifdef MOCKDB_APP_DEBUG_LOG
    std::cout << "[MOCKDB::app] CART " << session_id << " " << ss.str() << std::endl;
#endif // MOCKDB_APP_DEBUG_LOG

    return cart_list;
}

double shopping_cart::get_bill(long session_id) {

}

/*
 * Checks if item is already present in cart; if not present, it inserts into cart list.
 */
void shopping_cart::_add_item(int id, long session_id) {
    std::string user = std::to_string(user_id);
    web::json::value cart;
    try {
        cart = this->store->get("cart:" + user, session_id);
    } catch (mockdb::key_not_found_exception &e) {
        // no cart exists, create a new one and add item
        cart["items"] = web::json::value::array();
        cart["items"][0] = id;
        cart["count"] = web::json::value(1);
        this->store->put("cart:" + user, cart, session_id);
        return;
    }

    // Check if already present
    web::json::array items = cart["items"].as_array();
    for (web::json::array::iterator it = items.begin(); it != items.end(); it++) {
        if (*it == web::json::value(id)) {
            return;
        }
    }

    int item_count = cart["count"].as_integer();
    cart["items"][item_count] = id;
    cart["count"] = web::json::value(item_count + 1);
    this->store->put("cart:" + user, cart, session_id);
}

/*
 * If item already exists then it adds to existing quantity, otherwise just insert quantity.
 */
void shopping_cart::_add_quantity(int id, int quantity, long session_id) {
    int existing_quantity = _get_quantity(id, session_id);
    _change_quantity(id, existing_quantity + quantity, session_id);
}

int shopping_cart::_get_quantity(int id, long session_id) {
    int quantity = 0;
    web::json::value val;
    try {
        val = this->store->get("cart:" + std::to_string(this->user_id) +
                               ":" + std::to_string(id) + ":quantity", session_id);
        quantity = val["value"].as_integer();
    } catch (std::exception &e) {
        // Pass
    }
    return quantity;
}

void shopping_cart::_change_quantity(int id, int quantity, long session_id) {
    web::json::value val;
    val["value"] = web::json::value(quantity);
    this->store->put("cart:" + std::to_string(this->user_id) + ":" + std::to_string(id) +
                     ":quantity", val, session_id);
}

#endif //MOCK_KEY_VALUE_STORE_SHOPPING_CART_H
