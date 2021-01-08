// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Demo of how to use the kv store API

#include "../include/kv_store.h"
#include "../include/read_response_selector.h"

#include <string>

int main() {
    mockdb::kv_store<std::string, int> *store;
    mockdb::read_response_selector<std::string, int> *get_next_tx;

    // Initialize kv store and read response selector
    get_next_tx = new mockdb::causal_read_response_selector<std::string, int>();
    store = new mockdb::kv_store<std::string, int>(get_next_tx);
    get_next_tx->init_consistency_checker(store);

    // Put key-value pair in store
    store->put("a", 1);
    store->put("b", 2, 1);
    store->put("c", 3, 2);
    store->put("b", 4, 2);

    // Read from store
    std::cout << store->get("b", 1) << std::endl;
    std::cout << store->get("b", 2) << std::endl;
    std::cout << store->get("c", 1) << std::endl;
    // Output could be:
    // 2 4 3 or 4 4 3

    // Free memory
    delete store;
    delete get_next_tx;

    return 0;
}


