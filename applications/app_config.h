// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_APP_CONFIG_H
#define MOCK_KEY_VALUE_STORE_APP_CONFIG_H

enum consistency {linear, causal, k_causal};

struct app_config {
    char *log_file_name;
    int iterations;
    consistency consistency_level = consistency::linear;
    bool debug = false;
    bool random_test = false;
    int num_random_test = 1;

    ~app_config(){
    }
};
#endif //MOCK_KEY_VALUE_STORE_APP_CONFIG_H
