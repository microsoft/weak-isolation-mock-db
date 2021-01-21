// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "../utils.h"
#include "../../kv_store/include/read_response_selector.h"
#include "courseware.h"

#include <thread>

#define NUM_SESSIONS 2
#define NUM_OPS 3

app_config *config = nullptr;
mockdb::kv_store<std::string, web::json::value> *store;

// For fixed run
std::vector<int> assert_counter(6, 0);
std::vector<int> results = {0, 0, 0, 0};
std::vector<std::vector<int>> serial_results = {
        {1, 2, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {1, 1, 0, 0}
};

// For random run
std::vector<std::vector<int>> operations(NUM_SESSIONS);
std::vector<course> cr = {cs101, hs201, ph301};
std::vector<student> st = {DK, JV, RB};

void assert_count(bool result, int assert_index) {
    if (!result)
        assert_counter[assert_index]++;
}

void populate_courseware(courseware *courseware_app) {
    courseware_app->add_student(DK);
    courseware_app->add_student(JV);
    courseware_app->add_student(RB);
    courseware_app->add_course(cs101);
    courseware_app->open_course(cs101.get_id());
    courseware_app->add_course(hs201);
    courseware_app->open_course(hs201.get_id());
    courseware_app->add_course(ph301);
    courseware_app->open_course(ph301.get_id());
}


/*
 * Two concurrent enroll transactions may enroll students beyond the course capacity
 * A student may be enrolled in a course which is being concurrently removed
 * C1: ENROLL(CS101) | GET_ENROLLMENTS() | ENROLL(HS201) | GET_ENROLLMENTS()
 * C2: ENROLL(CS101) | GET_ENROLLMENTS() | REMOVE(HS201) | GET_ENROLLMENTS()
 * Serializable output:
 * {'CS101': [DK], 'HS201': []}, {'CS101': [DK], 'HS201': [DK]}, {'CS101': [DK], 'HS201': [DK]}, {'CS101': [DK]}
 * {'CS101': [DK], 'HS201': []}, {'CS101': [DK]}, {'CS101': [DK]}, {'CS101': [DK]}
 * {'CS101': [JV], 'HS201': []}, {'CS101': [JV], 'HS201': []}, {'CS101': [JV], 'HS201': [DK]}, {'CS101': [JV]}
 * {'CS101': [JV], 'HS201': []}, {'CS101': [JV], 'HS201': []}, {'CS101': [JV]}, {'CS101': [JV]}
 * {'CS101': [JV], 'HS201': []}, {'CS101': [JV]}, {'CS101': [JV]}, {'CS101': [JV]}
 */
void do_op(courseware *courseware_app, int t_id) {
    if (t_id == 1) {
        courseware_app->tx_start();
        courseware_app->enroll(DK.get_id(), cs101.get_id(), t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::map<int, std::vector<int>> courses = courseware_app->get_enrollments(t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        courseware_app->enroll(DK.get_id(), hs201.get_id(), t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::map<int, std::vector<int>> courses1 = courseware_app->get_enrollments(t_id);
        courseware_app->tx_end();

        for (auto e : courses) {
            if (e.first == cs101.get_id()) {
                for (auto s : e.second) {
                    if (s == DK.get_id())
                        results[0] = 1;
                }
            }
        }

        for (auto e : courses1) {
            for (auto s : e.second) {
                if (s == DK.get_id())
                    results[1]++;
            }
        }
    }
    else if (t_id == 2) {
        courseware_app->tx_start();
        courseware_app->enroll(JV.get_id(), cs101.get_id(), t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::map<int, std::vector<int>> courses = courseware_app->get_enrollments(t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        courseware_app->delete_course(hs201, t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::vector<int> students = courseware_app->get_enrolled_students(hs201.get_id(), t_id);
        courseware_app->tx_end();

        for (auto e : courses) {
            if (e.first == cs101.get_id()) {
                for (auto s : e.second) {
                    if (s == JV.get_id())
                        results[2] = 1;
                }
            }
        }

        for (auto e : students) {
            if (e == DK.get_id()) {
                results[3]++;
            }
        }
    }
}

void do_dfs_op(courseware *courseware_app, int t_id) {
    if (t_id == 1) {
        courseware_app->tx_start();
        courseware_app->enroll(DK.get_id(), cs101.get_id(), t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::map<int, std::vector<int>> courses = courseware_app->get_enrollments(t_id);
        courseware_app->tx_end();

    }
    else if (t_id == 2) {
        courseware_app->tx_start();
        courseware_app->enroll(JV.get_id(), cs101.get_id(), t_id);
        courseware_app->tx_end();

        courseware_app->tx_start();
        std::map<int, std::vector<int>> courses = courseware_app->get_enrollments(t_id);
        courseware_app->tx_end();

    }
}

void do_random_op(courseware *courseware_app, int t_id) {
    int idx = 0;
    for (int i = 0; i < operations[t_id - 1].size(); i++) {
        courseware_app->tx_start();
        if (operations[t_id - 1][i] == 0) {
            courseware_app->enroll(st[idx].get_id(), cr[idx].get_id(), t_id);
        }
        else if (operations[t_id - 1][i] == 1) {
            std::map<int, std::vector<int>> courses = courseware_app->get_enrollments(t_id);
        }
        else {
            courseware_app->delete_course(cr[idx], t_id);
        }
        courseware_app->tx_end();
        idx = (idx + 1) % cr.size();
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
            operations[i - 1][j - 1] = random_value % 3;
        }
    }
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

    courseware *courseware_app = new courseware(store, config->consistency_level);

    populate_courseware(courseware_app);
    std::vector<std::thread> threads;

    for (int i = 1; i <= NUM_SESSIONS; i++) {
        if (config->random_test)
            threads.push_back(std::thread(do_random_op, courseware_app, i));
        else
            threads.push_back(std::thread(do_op, courseware_app, i));
    }

    for (auto &t : threads)
        t.join();

    delete courseware_app;
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
            bool register_overflow = results[0] == 1 && results[2] == 1;
            assert_count(!register_overflow, 0);
            bool removed_register = (results[0] == 0 && results[1] == 1 && results[3] == 1) ||
                                    (results[1] == 2 && results[3] == 1 );
            assert_count(!removed_register, 1);

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

    int voilation_count = 0;
    for (auto e : assert_counter)   voilation_count += e;

    std::cout << "Total violations found: " << voilation_count
        << " in " << config->iterations << " iterations\n";
    delete config;
    return 0;
}
