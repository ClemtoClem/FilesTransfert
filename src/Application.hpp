#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "FileTransfer.hpp"
#include "Interface.hpp"
#include <string>
#include <vector>
#include <thread>
#include <atomic>

class Application {
public:
    Application();
    void run();

private:
    Interface ui;
    std::vector<std::string> received_files;
    std::atomic<bool> running;
    std::vector<std::string> search_for_computers();
    void server_mode();
    void client_mode(const std::string &address, const std::string &file);
    //void receive_file_thread(int new_socket);
    //void send_file_thread(const std::string &filename);
};

#endif // APPLICATION_HPP