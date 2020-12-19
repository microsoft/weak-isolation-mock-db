// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "utils.h"

#include <iostream>
#include <cstring>

app_config *parse_command_line(int argc, char **argv) {
    app_config *config      = new app_config();
    config->log_file_name   = argv[1];
    config->iterations      = atoi(argv[2]);
    char *consistency_arg   = argv[3];
    char *random_arg        = argv[4];
    config->num_random_test = atoi(argv[5]);

    if (strcmp(random_arg, "random") == 0) {
        std::cout << "random operations\n";
        config->random_test = true;
    }
    else if (strcmp(random_arg, "fixed") == 0) {
        std::cout << "fixed operations\n";
        config->random_test = false;
    }

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
