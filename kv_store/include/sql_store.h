// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Key-value store API


#ifndef MOCK_KEY_VALUE_STORE_SQL_STORE_H
#define MOCK_KEY_VALUE_STORE_SQL_STORE_H

//#define MOCKDB_DEBUG_LOG
#define DEFAULT_SESSION 1

#include "transaction.h"
#include "key_not_found_exception.h"
#include "query_parse_exception.h"
#include "consistency_exception.h"
#include "../sql_parser/src/SQLParser.h"
#include "../sql_parser/src/util/sqlhelper.h"

#include <list>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <sstream>

namespace mockdb {
    class sql_store : public kv_store<char*, char*> {
    public:
        sql_store(read_response_selector<char*, char*>* read_selector);
        virtual ~sql_store();

        // SQL interface
        int execute_query(std::string query, long session_id = DEFAULT_SESSION);

    private:
        void create_table(const hsql::CreateStatement* statement, long session_id);
        std::list<char*> select_from_table(const hsql::SelectStatement* statement, long session_id);
    };

}

mockdb::sql_store::sql_store(read_response_selector<char*, char*> *get_next_tx) 
    : kv_store<char*, char*>(get_next_tx) {
}

// Destructor
mockdb::sql_store::~sql_store() {
}

/*
* Executes the given query on the store.
* May throw query_parse_exception.
* For relational to kv API mapping, we need to store some metadata.
* For each table, the kv store maintains the list of columns and list of primary keys.
* Each column within the table can then be accessed by the key "table_name:primary_key:column_name".
* The current implementation assumes type <K,V> = <char*, char*>
*/
int mockdb::sql_store::execute_query(std::string query, long session_id) {
    // Parse the given query
    hsql::SQLParserResult parsed_queries;
    hsql::SQLParser::parse(query, &parsed_queries);

    // Check if the parsing was successful
    if (!parsed_queries.isValid()) {
        throw query_parse_exception(query);
    }

    for (auto i = 0u; i < parsed_queries.size(); ++i) {
        const hsql::SQLStatement* stmt = parsed_queries.getStatement(i);
        hsql::printStatementInfo(stmt);
        switch (stmt->type()) {
        case hsql::StatementType::kStmtSelect:
            select_from_table((const hsql::SelectStatement*)stmt, session_id);
            break;
        case hsql::StatementType::kStmtCreate:
            create_table((const hsql::CreateStatement*)stmt, session_id);
            break;
        default:
            break;
        }
    }
}

/*
* Create a new table on top of kv-store
* Stores column metadata in kv-store
* key "table_name:columns" contains the comma separated list of columns
*/
void mockdb::sql_store::create_table(const hsql::CreateStatement* statement, long session_id) {
    char* table_name = statement->tableName;
    char* key = strcat(table_name, ":columns");
    char* value = "";
    
    // TODO: Iterate through `columns` and append with comma to value
    
    this->put(key, value, session_id);
}

/*
* Select rows from the table. Only fetches the column specified in the query.
*/
std::list<char*> mockdb::sql_store::select_from_table(const hsql::SelectStatement* statement, long session_id) {
    std::list<char*> result;
    // TODO
    return result;
}

#endif //MOCK_KEY_VALUE_STORE_SQL_STORE_H
