// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "treiber_stack.h"
#include "../app_config.h"
#include "../utils.h"
#include "../../kv_store/include/read_response_selector.h"

#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>
#include <chrono>

#define NUM_SESSIONS 3
#define NUM_OPS 3

std::vector<std::vector<bool>> operations(NUM_SESSIONS);
std::vector<int> results(6, 0);
std::vector<int> assert_counter(6, 0);

app_config *config;

void assert_count(bool result, int assert_index) {
    if (result)
        assert_counter[assert_index]++;
}

// Checks if any pop returned a pushed value multiple times
void check_pop_valid() {
    std::unordered_set<int> unique_vals;
    bool repeat_value = false;
    for (auto e : results) {
        if (e != 0) {
            if (unique_vals.find(e) != unique_vals.end()) {
                repeat_value = true;
            }
            unique_vals.insert(e);
        }
    }
    assert_count(repeat_value, 0);

}

// Randomly choose operations for each thread
void random_fill(long seed) {
    for (int i = 1; i <= NUM_SESSIONS; i++) {
        long thread_seed = seed * 97 + 19;
        std::mt19937 generator (thread_seed);
        operations[i - 1].resize(NUM_OPS);
        for (int j = 1; j <= NUM_OPS; j++) {
            unsigned long long random_value = generator();
            operations[i - 1][j - 1] = random_value % 2;
        }
    }
}

void do_operations(treiber_stack<int> *stack, int t_id) {
    std::vector<int> pop_values;
    for (size_t i = 1; i <= operations[t_id - 1].size(); i++) {
        if (operations[t_id - 1][i - 1]) {
            stack->push(t_id * 10 + i, t_id);
        }
        else {
            pop_values.push_back(stack->pop(t_id));
        }
    }
    if (!config->random_test) {
        for (size_t i = 0; i < pop_values.size(); i++)
            results[(t_id - 1) * 2 + i] = pop_values[i];
    }
}
/*
 */
void run_iteration() {
    mockdb::read_response_selector<long, std::pair<int, long>> *get_next_tx;

    if (config->consistency_level == consistency::causal)
        get_next_tx = new mockdb::causal_read_response_selector<long, std::pair<int, long>>();
    else if (config->consistency_level == consistency::linear)
        get_next_tx = new mockdb::linearizable_read_response_selector<long, std::pair<int, long>>();
    else if (config->consistency_level == consistency::k_causal)
        get_next_tx = new mockdb::k_causal_read_response_selector<long, std::pair<int, long>>(2,
                                                                                              operations.size() * operations[0].size()/2);
    else
        get_next_tx = new mockdb::causal_read_response_selector<long, std::pair<int, long>>();

    mockdb::kv_store<long, std::pair<int, long>> *store = new mockdb::kv_store<long, std::pair<int, long>>(get_next_tx);
    get_next_tx->init_consistency_checker(store);

    treiber_stack<int> *stack = new treiber_stack<int>(store);
    std::vector<std::thread> threads;

    for (int i = 1; i <= NUM_SESSIONS; i++) {
        threads.push_back(std::thread(do_operations, stack, i));
    }

    for (auto &t : threads)
        t.join();

    delete store;
    delete get_next_tx;
}

/*
 * Args:
 * num of iterations
 * consistency-level: linear, causal, k-causal
 */
int main(int argc, char **argv) {
    config = parse_command_line(argc, argv);
    std::cout << "Running stack application\n";

    if (!config->random_test){
        // T1: PUSH PUSH PUSH
        // T2: POP POP POP
        operations = {
                {1, 1, 0},
                {0, 1, 0},
                {0, 0}
        };
    }

    for (int i = 1; i <= config->num_random_test; i++) {

        if (config->random_test) {
            if (config->debug)
                std::cout << "[MOCKDB::app] RANDOM " << i - 1 << " start" << std::endl;

            random_fill(i * 19 + 11);
        }

        for (int j = 0; j < config->iterations; j++) {
            if (config->debug)
                std::cout << "[MOCKDB::app] Iteration " << j << " start" << std::endl;

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            run_iteration();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            check_pop_valid();

            results[0] = results[1] = results[2] = results[3] = 0;

            std::stringstream ss;
            for (auto k : assert_counter)
                ss << k << " ";

            if (config->debug) {
                std::cout << "[MOCKDB::app] " << ss.str() << std::endl;
                std::cout << "[MOCKDB::app] Iteration " << j << " end "
                    << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                    << std::endl;
            }
        }

        if (config->random_test && config->debug) {
            std::cout << "[MOCKDB::app] RANDOM " << i - 1 << " end" << std::endl;
        }
    }

    int voilation_count = assert_counter[0];

    std::cout << "Total violations found: " << voilation_count
        << " in " << config->iterations << " iterations\n";

    delete config;

    return 0;
}
