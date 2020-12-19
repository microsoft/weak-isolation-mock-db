// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_STUDENT_H
#define MOCK_KEY_VALUE_STORE_STUDENT_H

#include <string>

class student {
public:
    student(int id, std::string name, bool registered, std::string roll_number) {
        this->id = id;
        this->name = name;
        this->registered = registered;
        this->roll_number = roll_number;
    }

    int get_id() {
        return id;
    }
    std::string get_name() {
        return name;
    }
    bool is_registered() {
        return registered;
    }

    std::string get_roll_number() {
        return roll_number;
    }

private:
    int id;
    std::string name;
    bool registered;
    std::string roll_number;
};

student DK(1, "dip", true, "1");
student JV(2, "JV", true, "2");
student RB(3, "RB", true, "3");

#endif //MOCK_KEY_VALUE_STORE_STUDENT_H
