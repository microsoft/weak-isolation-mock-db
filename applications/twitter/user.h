// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_USER_H
#define MOCK_KEY_VALUE_STORE_USER_H

#include<vector>
#include <string>
#include "tweet.h"

class user {
public:
    user(long user_id, std::string username);
    std::string get_username();
    long get_id();

private:
    std::string username;
    long user_id;
};

user::user(long user_id, std::string username) {
    this->user_id = user_id;
    this->username = username;
}

std::string user::get_username() {
    return username;
}

long user::get_id() {
    return user_id;
}

user a(1, "a");
user b(2, "b");
user c(3, "c");

#endif //MOCK_KEY_VALUE_STORE_USER_H
