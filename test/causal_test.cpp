// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "kv_store.h"
#include "read_response_selector.h"

#include <cassert>

class causal_tests {

public:
    // Default ctor
    causal_tests() {

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

    void test_simple_read_write();
    void test_causal_read_write();

private:
    mockdb::kv_store<std::string, int> *store;
    mockdb::read_response_selector<std::string, int> *read_selector;
};

void causal_tests::test_simple_read_write() {
    int session_id = 123;
    store->put("a", 50, session_id);
    assert(store->get("a", session_id) == 50);

    store->put("b", 100, session_id);
    store->put("c", 150, session_id);
    assert(store->get("b", session_id) == 100);
    assert(store->get("c", session_id) == 150);
}

/*
 * C1: W(x)a
 * C2:         R(x)a   W(x)b
 * C3:                         R(x)b   R(x)a   NOT OK
 * C4:                         R(x)a   R(x)b   OK
 */
void causal_tests::test_causal_read_write() {
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

    std::cout << c3_rx1 << " " << c3_rx2 << std::endl;
    std::cout << c4_rx1 << " " << c4_rx2 << std::endl;

    if (c3_rx1 == b)
        assert(c3_rx2 != a);
    if (c4_rx1 == b)
        assert(c4_rx2 != a);
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
    causal_tests ct;

    for (int i = 0; i < test_count; i++) {
        ct.SetUp();
        ct.test_simple_read_write();
        ct.test_causal_read_write();
        ct.TearDown();
    }

    std::cout << "All causal test passed!\n";
}