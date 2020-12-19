// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_COURSE_H
#define MOCK_KEY_VALUE_STORE_COURSE_H

#include <string>

class course {
public:
    course(int id, std::string name, std::string department, std::string status, int capacity) {
        this->id = id;
        this->name = name;
        this->department = department;
        this->status = status;
        this->capacity = capacity;
    }

    int get_id() {
        return id;
    }

    std::string get_name() {
        return name;
    }

    std::string get_department() {
        return department;
    }

    std::string get_status() {
        return status;
    }

    int get_capacity() {
        return capacity;
    }

private:
    int id;
    std::string name;
    std::string department;
    std::string status;
    int capacity;
};

course cs101(1, "Introduction to Computer Science", "CS", "close", 1);
course hs201(2, "Happiness and Well Being", "HS", "close", 2);
course ph301(3, "Meaning of Life", "PH", "close", 1);

#endif //MOCK_KEY_VALUE_STORE_COURSE_H
