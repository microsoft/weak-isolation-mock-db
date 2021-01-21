// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "../utils.h"
#include "../../kv_store/include/read_response_selector.h"
#include "twitter.h"

#include <thread>
#include <ctime>

#define NUM_SESSIONS 3
#define NUM_OPS 3

app_config *config = nullptr;
mockdb::kv_store<std::string, web::json::value> *store;
std::vector<int> assert_counter(6, 0);
std::vector<int> results(2, 0);

// For random run
std::vector<std::vector<int>> operations(NUM_SESSIONS);
std::vector<user> users = {a, b, c};

/*
 */
void populate_twitter(twitter *twitter_store) {
    twitter_store->add_user(a);
    twitter_store->add_user(b);
    twitter_store->add_user(c);
}

void assert_count(bool result, int assert_index) {
    if (result)
        assert_counter[assert_index]++;
}


/*
 * A: TWEET | TWEET
 * B: FOLLOW(A) | NEWSFEED
 * B: TIMELINE(A)
 *
 * Tweets of recently followed user not showing up in newsfeed, even after viewing those
 * tweets through another session.
 */
void do_op(twitter *twitter_store, int t_id) {
    if (t_id == 1) {
        twitter_store->tx_start();
        twitter_store->publish_tweet(a, tweet(1, "hello", 1));
        twitter_store->tx_end();

        twitter_store->tx_start();
        twitter_store->publish_tweet(a, tweet(2, "world", 2));
        twitter_store->tx_end();
    }
    else if (t_id == 2) {
        twitter_store->tx_start();
        twitter_store->follow(b, a);
        twitter_store->tx_end();

        twitter_store->tx_start();
        std::vector<tweet> newsfeed = twitter_store->get_newsfeed(b);
        twitter_store->tx_end();

        results[1] = newsfeed.size();
    }
    else if (t_id == 3) {
        twitter_store->tx_start();
        std::vector<tweet> timeline = twitter_store->get_timeline(a, t_id);
        twitter_store->tx_end();

        results[0] = timeline.size();
    }
}

// Fills operations matrix randomly
void random_fill(long seed) {
    for (int i = 1; i <= NUM_SESSIONS; i++) {
        long thread_seed = seed * 97 + 19;
        std::mt19937 generator (thread_seed);
        operations[i - 1].resize(NUM_OPS);
        for (int j = 1; j <= NUM_OPS; j++) {
            unsigned long long random_value = generator();
            operations[i - 1][j - 1] = random_value % 4;
        }
    }
}

/*
 * Choices of operations:
 * 0 : FOLLOW
 * 1 : NEWSFEED
 * 2 : TWEET
 */
void do_random_op(twitter *twitter_store, int t_id) {
    int idx = 0;
    for (int j = 1; j <= NUM_OPS; j++) {
        twitter_store->tx_start();
        if (operations[t_id - 1][j - 1] == 0) {
            twitter_store->follow(users[idx], users[(idx + 1) % users.size()]);
        }
        else if (operations[t_id - 1][j - 1] == 1) {
            twitter_store->get_newsfeed(users[idx]);
        }
        else if (operations[t_id - 1][j - 1] == 2 || operations[t_id - 1][j - 1] == 3) {
            std::string tweet_msg("Hello world! " + std::to_string(t_id));
            twitter_store->publish_tweet(users[idx], tweet(t_id * 19 + j,
                                                           tweet_msg,
                                                           std::time(0)));
        }
        twitter_store->tx_end();
        idx = (idx + 1) % users.size();
    }
    // Last operation is fixed - a read operation
    twitter_store->tx_start();
    twitter_store->get_all_timeline(t_id);
    twitter_store->tx_end();
}

void run_iteration() {
    mockdb::read_response_selector<std::string, web::json::value> *get_next_tx;

    if (config->consistency_level == consistency::causal)
        get_next_tx = new mockdb::causal_read_response_selector<std::string, web::json::value>();
    else if (config->consistency_level == consistency::linear)
        get_next_tx = new mockdb::linearizable_read_response_selector<std::string, web::json::value>();
    else if (config->consistency_level == consistency::k_causal)
        get_next_tx = new mockdb::k_causal_read_response_selector<std::string, web::json::value>(2, 12);

    store = new mockdb::kv_store<std::string, web::json::value>(get_next_tx);
    get_next_tx->init_consistency_checker(store);

    twitter *twitter_store = new twitter(store, config->consistency_level);
    populate_twitter(twitter_store);

    std::vector<std::thread> threads;

    for (int i = 1; i <= NUM_SESSIONS; i++) {
        if (config->random_test)
            threads.push_back(std::thread(do_random_op, twitter_store, i));
        else
            threads.push_back(std::thread(do_op, twitter_store, i));
    }

    for (auto &t : threads)
        t.join();

    delete twitter_store;
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

    for (int i = 1; i <= config->num_random_test; i++) {

        if (config->random_test) {
            if (config->debug)
                std::cout << "[MOCKDB::app] RANDOM " << i - 1 << " start" << std::endl;

            random_fill(i * 19 + 11);
        }

        for(int j = 0; j < config->iterations; j++) {
            if (config->debug)
                std::cout << "[MOCKDB::app] Iteration " << j << " start" << std::endl;

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            run_iteration();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            assert_count(results[0] > results[1], 0);

            results[0] = results[1] = 0;

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
