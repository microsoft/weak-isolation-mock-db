// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "utils.h"

#include <iostream>
#include <cstring>

app_config *parse_command_line(int , char **argv) {
    app_config *config      = new app_config();
    config->iterations      = atoi(argv[1]);
    char *consistency_arg   = argv[2];
    config->random_test     = false;
    config->num_random_test = 1;

    if (strcmp(consistency_arg, "linear") == 0) {
        std::cout << "linearizability consistency\n";
        config->consistency_level = consistency::linear;
    }
    else if (strcmp(consistency_arg, "causal") == 0) {
        std::cout << "causal consistency\n";
        config->consistency_level = consistency::causal;
    }
    else if (strcmp(consistency_arg, "k-causal") == 0) {
        std::cout << "k-causal consistency\n";
        config->consistency_level = consistency::k_causal;
    }

    return config;
}
