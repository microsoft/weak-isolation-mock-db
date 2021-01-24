// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "kv_store.h"
#include "read_response_selector.h"

#include <cassert>

class etag_tests {

public:
    // Default ctor
    etag_tests() {

    }

    // Called once before each test
    virtual void SetUp()
    {
        read_selector = new mockdb::causal_read_response_selector<std::string, int>();
        store = new mockdb::kv_store<std::string, int>(read_selector);
        read_selector->init_consistency_checker(store);
    }

    // Called once before each test
    virtual void TearDown() {
        delete read_selector;
        delete store;
    }

    void test_version_number();

private:
    mockdb::kv_store<std::string, int> *store;
    mockdb::read_response_selector<std::string, int> *read_selector;
};

void etag_tests::test_version_number() {
    int session_id_a = 123, session_id_b = 345;
    store->put("key1", 50, session_id_a);
    std::pair<int, size_t> read1 = store->get_with_version("key1");
    assert(read1.first == 50);
    assert(read1.second == 1);

    store->put("key1", 100, session_id_b);
    std::pair<int, size_t> read2 = store->get_with_version("key1");

    if (read2.first == 50) {
        assert(read2.second == 1);
    }
    else if (read2.first == 100) {
        assert(read2.second == 2);
    }
    else {
        assert(false);
    }
}

/*
 * Args:
 * num-test : number of times to run test
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid arguments, specify number of times to run test\n";
        return -1;
    }

    int test_count = atoi(argv[1]);
    etag_tests et;

    for (int i = 0; i < test_count; i++) {
        et.SetUp();
        et.test_version_number();
        et.TearDown();
    }

    std::cout << "All etag tests passed!\n";
}