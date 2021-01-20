// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_COURSEWARE_H
#define MOCK_KEY_VALUE_STORE_COURSEWARE_H

#include "course.h"
#include "student.h"
#include "../utils.h"
#include "../../kv_store/include/kv_store.h"

#include <cpprest/json.h>
#include <thread>
#include <map>

/*
 * students : {
 *              "list": [
 *                          xxx,
 *                          xxx,
 *                      ],
 *               "count": xxx
 *            }
 *
 * courses : {
 *              "list": [
 *                          xxx,
 *                          xxx,
 *                      ],
 *              "count": xxx
 *           }
 *
 * student:id : {
 *                  "name" : "xxx",
 *                  "registered" : true/false,
 *                  "roll_number" : xxx
 *              }
 *
 * course:id :  {
 *                   "name" : "xxx",
 *                   "department" : "xxx",
 *                   "status" : "open/close",
 *                   "capacity" : xxx
 *              }
 *
 * enrollment:student:id: {
 *                              "list": [
 *                                          xxx,
 *                                      ],
 *                               "count" : xxx
 *                        }
 *
 * enrollment:course:id: {
 *                              "list": [
 *                                          xxx,
 *                                      ],
 *                               "count" : xxx
 *                        }
 */

class courseware {
public:
    courseware(mockdb::kv_store<std::string, web::json::value> *store,
               consistency consistency_level);

    void tx_start();
    void tx_end();

    void add_student(student s, long session_id = 0);
    void delete_student(student s, long session_id = 0);
    void add_course(course c, long session_id = 0);
    void delete_course(course c, long session_id = 0);

    void register_student(int student_id, long session_id = 0);
    void deregister_student(int student_id, long session_id = 0);
    void open_course(int course_id, long session_id = 0);
    void close_course(int course_id, long session_id = 0);

    void enroll(int student_id, int course_id, long session_id = 0);
    std::vector<int> get_enrolled_courses(int student_id, long session_id = 0);
    std::vector<int> get_enrolled_students(int course_id, long session_id = 0);
    std::map<int, std::vector<int>> get_enrollments(long session_id = 0);

private:
    mockdb::kv_store<std::string, web::json::value> *store;
    consistency consistency_level;
    std::mutex mtx;
};

courseware::courseware(mockdb::kv_store<std::string, web::json::value> *store,
                       consistency consistency_level) {
    this->store = store;
    this->consistency_level = consistency_level;
}

void courseware::tx_start() {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 4));
    mtx.lock();
}

void courseware::tx_end() {
    mtx.unlock();
}

void courseware::add_student(student s, long session_id) {
    // Insert student into list
    web::json::value students;
    try {
        students = store->get("students", session_id);
    } catch (std::exception &e) {
        // students list doesn't exist, create a new one
        students[L"list"] = web::json::value::array();
        students[L"count"] = web::json::value(0);
    }

    int students_count = students[L"count"].as_integer();
    students[L"list"][students_count] = s.get_id();
    students[L"count"] = web::json::value(students_count + 1);

    store->put("students", students, session_id);

    // Add student detail
    web::json::value student_details;
    student_details[L"name"] = web::json::value(utility::conversions::to_string_t(s.get_name()));
    student_details[L"registered"] = web::json::value(s.is_registered());
    student_details[L"roll_number"] = web::json::value(utility::conversions::to_string_t(s.get_roll_number()));

    store->put("student:" + std::to_string(s.get_id()), student_details, session_id);

    // Add enrollment details
    web::json::value course_list;
    course_list[L"list"] = web::json::value::array();
    course_list[L"count"] = web::json::value(0);
    store->put("enrollment:student:" + std::to_string(s.get_id()), course_list, session_id);
}

void courseware::delete_student(student s, long session_id) {
    deregister_student(s.get_id(), session_id);

    // Remove from the student list
    web::json::value students;
    try {
        students = this->store->get("students", session_id);
    } catch (std::exception &e) {
        return;
    }

    web::json::array students_list = students[L"list"].as_array();
    std::vector<web::json::value> updated_list;
    for (web::json::array::iterator it = students_list.begin(); it != students_list.end(); it++) {
        if (*it != web::json::value(s.get_id())) {
            updated_list.push_back(*it);
        }
    }

    students[L"list"] = web::json::value::array(updated_list);
    students[L"count"] = web::json::value(updated_list.size());
    this->store->put("students", students, session_id);

    // Removed enrolled courses
    students[L"list"] = web::json::value::array();
    students[L"count"] = web::json::value(0);
    store->put("enrollment:student:" + std::to_string(s.get_id()), students, session_id);
}

void courseware::add_course(course c, long session_id) {
    // Insert course into list
    web::json::value courses;
    try {
        courses = store->get("courses", session_id);
    } catch (std::exception &e) {
        // courses list doesn't exist, create a new one
        courses[L"list"] = web::json::value::array();
        courses[L"count"] = web::json::value(0);
    }

    int courses_count = courses[L"count"].as_integer();
    courses[L"list"][courses_count] = c.get_id();
    courses[L"count"] = web::json::value(courses_count + 1);

    store->put("courses", courses, session_id);

    // Add course detail
    web::json::value course_details;
    course_details[L"name"] = web::json::value(utility::conversions::to_string_t(c.get_name()));
    course_details[L"department"] = web::json::value(utility::conversions::to_string_t(c.get_department()));
    course_details[L"status"] = web::json::value(utility::conversions::to_string_t(c.get_status()));
    course_details[L"capacity"] = web::json::value(c.get_capacity());

    store->put("course:" + std::to_string(c.get_id()), course_details, session_id);

    // Add enrollment details
    web::json::value course_enrollment;
    course_enrollment[L"list"] = web::json::value::array();
    course_enrollment[L"count"] = web::json::value(0);
    store->put("enrollment:course:" + std::to_string(c.get_id()), course_enrollment, session_id);
}

void courseware::delete_course(course c, long session_id) {
    close_course(c.get_id(), session_id);

    // Remove from the course list
    web::json::value courses;
    try {
        courses = this->store->get("courses", session_id);
    } catch (std::exception &e) {
        return;
    }

    web::json::array courses_list = courses[L"list"].as_array();
    std::vector<web::json::value> updated_list;
    for (web::json::array::iterator it = courses_list.begin(); it != courses_list.end(); it++) {
        if (*it != web::json::value(c.get_id())) {
            updated_list.push_back(*it);
        }
    }

    courses[L"list"] = web::json::value::array(updated_list);
    courses[L"count"] = web::json::value(updated_list.size());
    this->store->put("courses", courses, session_id);

    // Removed enrolled students
    courses[L"list"] = web::json::value::array();
    courses[L"count"] = web::json::value(0);
    store->put("enrollment:course:" + std::to_string(c.get_id()), courses, session_id);
}

void courseware::register_student(int student_id, long session_id) {
    web::json::value student_details;
    try {
        student_details = store->get("student:" + std::to_string(student_id), session_id);
    } catch (std::exception &e) {
        // student doesn't exist
        return;
    }
    student_details[L"registered"] = web::json::value(true);
    store->put("student:" + std::to_string(student_id), student_details, session_id);
}

void courseware::deregister_student(int student_id, long session_id) {
    web::json::value student_details;
    try {
        student_details = store->get("student:" + std::to_string(student_id), session_id);
    } catch (std::exception &e) {
        // student doesn't exist
        return;
    }
    student_details[L"registered"] = web::json::value(0);
    store->put("student:" + std::to_string(student_id), student_details, session_id);
}

void courseware::open_course(int course_id, long session_id) {
    web::json::value course_details;
    try {
        course_details = store->get("course:" + std::to_string(course_id), session_id);
    } catch (std::exception &e) {
        // course doesn't exist
        return;
    }
    course_details[L"status"] = web::json::value("open");
    store->put("course:" + std::to_string(course_id), course_details, session_id);
}

void courseware::close_course(int course_id, long session_id) {
    web::json::value course_details;
    try {
        course_details = store->get("course:" + std::to_string(course_id), session_id);
    } catch (std::exception &e) {
        // course doesn't exist
        return;
    }
    course_details[L"status"] = web::json::value("close");
    store->put("course:" + std::to_string(course_id), course_details, session_id);
}

void courseware::enroll(int student_id, int course_id, long session_id) {
    // Verify student and course are registered and open
    web::json::value student_details, course_details;
    try {
        student_details = store->get("student:" + std::to_string(student_id), session_id);
        course_details = store->get("course:" + std::to_string(course_id), session_id);
    } catch (std::exception &e) {
        // student or course doesn't exist
        return;
    }
    if (student_details[L"registered"].as_bool() == 0 || course_details[L"status"].as_string() != utility::conversions::to_string_t("open"))
        return;

    // Retrieve list of students enrolled for the course
    web::json::value course_enrollment;
    try {
        course_enrollment = store->get("enrollment:course:" + std::to_string(course_id), session_id);
    } catch (std::exception &e) {
        // course enrollment list doesn't exist, create a new one
        course_enrollment[L"list"] = web::json::value::array();
        course_enrollment[L"count"] = web::json::value(0);
    }

    int student_count = course_enrollment[L"count"].as_integer();

    // Check enrollment count doesn't exceed the capacity of course
    if (student_count >= course_details[L"capacity"].as_integer()) {
        close_course(course_id, session_id);
        return;
    }

    // Add enrollment entry for course
    course_enrollment[L"list"][student_count] = student_id;
    course_enrollment[L"count"] = web::json::value(student_count + 1);

    store->put("enrollment:course:" + std::to_string(course_id), course_enrollment, session_id);

    // Add enrollment entry for student
    web::json::value student_enrollment;
    try {
        student_enrollment = store->get("enrollment:student:" + std::to_string(student_id), session_id);
    } catch (std::exception &e) {
        // student enrollment list doesn't exist, create a new one
        student_enrollment[L"list"] = web::json::value::array();
        student_enrollment[L"count"] = web::json::value(0);
    }

    int course_count = student_enrollment[L"count"].as_integer();
    student_enrollment[L"list"][course_count] = course_id;
    student_enrollment[L"count"] = web::json::value(course_count + 1);

    store->put("enrollment:student:" + std::to_string(student_id), student_enrollment, session_id);
}

std::vector<int> courseware::get_enrolled_courses(int student_id, long session_id) {
    std::vector<int> courses_enrolled;
    web::json::value student_enrollment;
    try {
        student_enrollment = store->get("enrollment:student:" + std::to_string(student_id), session_id);
    } catch (std::exception &e) {
        // student enrollment list doesn't exist
        return courses_enrolled;
    }
    web::json::array courses_list = student_enrollment[L"list"].as_array();
    for (web::json::array::iterator it = courses_list.begin(); it != courses_list.end(); it++) {
        courses_enrolled.push_back((*it).as_integer());
    }
    return courses_enrolled;
}

std::vector<int> courseware::get_enrolled_students(int course_id, long session_id) {
    std::vector<int> students_enrolled;
    web::json::value students_enrolled_json;
    try {
        students_enrolled_json = store->get("enrollment:course:" + std::to_string(course_id), session_id);
    } catch (std::exception &e) {
        // course enrollment list doesn't exist
        return students_enrolled;
    }
    web::json::array students_list = students_enrolled_json[L"list"].as_array();
    for (web::json::array::iterator it = students_list.begin(); it != students_list.end(); it++) {
        students_enrolled.push_back((*it).as_integer());
    }
    return students_enrolled;
}


std::map<int, std::vector<int>> courseware::get_enrollments(long session_id) {
    std::map<int, std::vector<int>> enrollments;

    web::json::value courses;
    try {
        courses = store->get("courses", session_id);
    } catch (std::exception &e) {
        // courses list doesn't exist
        return enrollments;
    }
    web::json::array course_list = courses[L"list"].as_array();
    for (web::json::array::iterator it = course_list.begin(); it != course_list.end(); it++) {
        enrollments[(*it).as_integer()] = get_enrolled_students((*it).as_integer(), session_id);
    }
    std::stringstream ss;
    for (auto e : enrollments) {
        ss << "(" << e.first << ": ";
        for (int i = 0; i < e.second.size(); i++) {
            ss << e.second[i];
            if (i != e.second.size() - 1)
                ss << " ";
        }
        ss << ") ";
    }

#ifdef MOCKDB_APP_DEBUG_LOG
    std::cout << "[MOCKDB::app] " << ss.str() << std::endl;
#endif // MOCKDB_APP_DEBUG_LOG

    return enrollments;
}
#endif //MOCK_KEY_VALUE_STORE_COURSEWARE_H
