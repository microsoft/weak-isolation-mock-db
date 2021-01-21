// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "../app_config.h"
#include "../utils.h"
#include "../../kv_store/include/read_response_selector.h"

#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>

#define NUM_SESSIONS 3
#define NUM_OPS 3

/*
 * Simple app does PUT and GET to kv-store through multiple sessions (threads).
 */
std::vector<std::vector<std::pair<std::string, int>>> operations(NUM_SESSIONS,
                                                                 std::vector<std::pair<std::string, int>>(NUM_OPS));

std::vector<std::string> keys = {"a", "b"};

app_config *config;

void do_operations(mockdb::kv_store<std::string, int> *store, int t_id) {
    for (int i = 1; i <= NUM_OPS; i++) {
        std::string key = operations[t_id - 1][i - 1].first;
        int value = operations[t_id - 1][i - 1].second;
        if (value == -1) {
            try {
                store->get(key, t_id);
            } catch (std::exception &e) {
                // Pass
            }
        }
        else {
            store->put(key, value, t_id);
        }
    }
}
/*
 */
void run_iteration() {
    mockdb::read_response_selector<std::string, int> *get_next_tx;

    if (config->consistency_level == consistency::causal)
        get_next_tx = new mockdb::causal_read_response_selector<std::string, int>();
    else if (config->consistency_level == consistency::linear)
        get_next_tx = new mockdb::linearizable_read_response_selector<std::string, int>();
    else if (config->consistency_level == consistency::k_causal)
        get_next_tx = new mockdb::k_causal_read_response_selector<std::string, int>(2,
                                                                                    operations.size() * operations[0].size()/2);

    mockdb::kv_store<std::string, int> *store = new mockdb::kv_store<std::string, int>(get_next_tx);
    get_next_tx->init_consistency_checker(store);

    std::vector<std::thread> threads;

    for (int i = 1; i <= NUM_SESSIONS; i++) {
        threads.push_back(std::thread(do_operations, store, i));
    }

    for (auto &t : threads)
        t.join();

    delete store;
    delete get_next_tx;
}

/*
 * Args:
 * log_filename
 * num of iterations
 * consistency-level: linear, causal, k-causal
 */
int main(int argc, char **argv) {
    config = parse_command_line(argc, argv);

    // T1: PUT(a) GET(a) PUT(b)
    // T2: PUT(a) PUT(b) GET(b)
    // T3: GET(a) GET(b) PUT(b)
    operations = {
            {{"a", 10}, {"a", -1}, {"b", 12}},
            {{"a", 20}, {"b", 21}, {"b", -1}},
            {{"a", -1}, {"b", -1}, {"b", 32}}
    };

    for(int j = 0; j < config->iterations; j++) {

        if (config->debug)
            std::cout << "[MOCKDB::app] Iteration " << j << " start" << std::endl;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        run_iteration();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        if (config->debug)
            std::cout << "[MOCKDB::app] Iteration " << j << " end "
                     << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                     << std::endl;

    }

    delete config;
    return 0;
}