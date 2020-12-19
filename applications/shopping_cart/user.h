// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_USER_H
#define MOCK_KEY_VALUE_STORE_USER_H

#include <string>

class user {
public:
    int id;
    std::string name;

    user(std::string name, int id) {
        this->name = name;
        this->id = id;
    }
};
#endif //MOCK_KEY_VALUE_STORE_USER_H
