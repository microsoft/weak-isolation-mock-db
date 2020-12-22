// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#include "http_server.h"

#include <iostream>
#include <cpprest/http_listener.h>


/*
 * Initializes the HTTP server on the given address.
 */
std::unique_ptr<mockdb::http_server<std::string, web::json::value>> init_server(const utility::string_t& address) {
    web::uri_builder uri(address);
    utility::string_t addr = uri.to_uri().to_string();
    std::unique_ptr<mockdb::http_server<std::string, web::json::value>> server = std::unique_ptr<mockdb::http_server<std::string, web::json::value>>(
                                                    new mockdb::http_server<std::string, web::json::value>(addr));
    server->open().wait();
    std::cout << "[MOCKDB::kvstore] Started HTTP server at: " <<   addr << std::endl;
    return std::move(server);
}

/*
 * Shut down the server.
 */
void close_server(std::unique_ptr<mockdb::http_server<std::string, web::json::value>> &server) {
    if (server != nullptr)
        server->close().wait();
}

/*
 * Starts HTTP server.
 * Arguments:
 * Port number
 * Log file path
 */
int main(int argc, char* argv[]) {
    utility::string_t port = U(argv[1]);
    utility::string_t address = U("http://127.0.0.1:");
    address.append(port);
    std::unique_ptr<mockdb::http_server<std::string, web::json::value>> server = init_server(address);

    while (true) {
        std::cout << "Type exit to stop" << std::endl;
        sleep(10);
        std::string line;
        std::getline(std::cin, line);
        if (line.compare("exit") == 0) {
            close_server(server);
            return 0;
        }
    }
}
