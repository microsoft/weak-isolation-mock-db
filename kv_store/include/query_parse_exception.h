// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_QUERY_PARSE_EXCEPTION_H
#define MOCK_KEY_VALUE_STORE_QUERY_PARSE_EXCEPTION_H

#include <exception>
#include <string>
#include <cstring>

namespace mockdb {
    class query_parse_exception : public std::exception {
    public:
        query_parse_exception(const std::string &query) {
            this->query = query;
        }

        const char* what() const throw() {
            return strcat((char *)"Error while parsing query ", this->query.c_str());
        }
    private:
        std::string query;
    };
}
#endif //MOCK_KEY_VALUE_STORE_QUERY_PARSE_EXCEPTION_H
