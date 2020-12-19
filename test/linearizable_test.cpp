// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "kv_store.h"
#include "read_response_selector.h"

#include <cassert>

class linearizable_tests {

public:

    linearizable_tests() {
        this->linearizable_count = 0;
        this->test_count = 0;
    }

    // Called once before each test
    virtual void SetUp()
    {
        read_selector = new mockdb::causal_read_response_selector<std::string, int>();
        store = new mockdb::kv_store<std::string, int>(read_selector);
        read_selector->init_consistency_checker(store);
    }

    virtual void TearDown() {
        delete read_selector;
        delete store;
    }

    // Called only once at the end of all test
    void TearDownTestSuite() {
        std::cout << "Linearizable: " << linearizable_count << " Total: " << test_count << std::endl;
        std::cout << "Percentage: " << (linearizable_count + 0.0) / test_count * 100.0 << std::endl;
    }

    void test_linearizability();

private:

    mockdb::kv_store<std::string, int> *store;
    mockdb::read_response_selector<std::string, int> *read_selector;
    long test_count, linearizable_count;
};


/*
 * C1: W(x)a
 * C2:         R(x)a   W(x)b
 * C3:                         R(x)b   R(x)b   OK
 * C4:                         R(x)b   R(x)b   OK
 */
void linearizable_tests::test_linearizability() {
    int c1 = 1, c2 = 2, c3 = 3, c4 = 4;
    int a = 5, b = 10;

    store->put("x", a, c1);
    int c2_rx1 = store->get("x", c2);
    store->put("x", b, c2);
    int c3_rx1 = store->get("x", c3);
    int c4_rx1 = store->get("x", c4);
    int c3_rx2 = store->get("x", c3);
    int c4_rx2 = store->get("x", c4);

    assert(c2_rx1 == a);

    if (c3_rx1 == b && c3_rx2 == b && c4_rx1 == b && c4_rx2 == b)
        this->linearizable_count++;

    this->test_count++;

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
    linearizable_tests lt;

    for (int i = 0; i < test_count; i++) {
        lt.SetUp();
        lt.test_linearizability();
        lt.TearDown();
    }

    lt.TearDownTestSuite();

    std::cout << "All linearizability test passed!\n";
}