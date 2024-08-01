#ifndef FILE_TRANSFER_HPP
#define FILE_TRANSFER_HPP

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <functional>

class FileTransfer {
public:
    static void send_file(const std::string &filename, int sock, std::function<void(const std::string&)> log_callback);
    static void receive_file(int sock, std::function<void(const std::string&)> log_callback);
    static void send_stop_signal(int sock);
};

#endif // FILE_TRANSFER_HPP
