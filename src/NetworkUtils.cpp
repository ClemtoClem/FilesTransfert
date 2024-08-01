#include "NetworkUtils.hpp"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <iostream>

std::vector<std::string> get_local_ip_addresses() {
    std::vector<std::string> ips;
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) {
                ips.push_back(host);
            }
        }
    }

    freeifaddrs(ifaddr);
    return ips;
}

std::vector<std::string> discover_instances(int port, int max_retries) {
    std::vector<std::string> discovered_ips;
    std::vector<std::string> local_ips = get_local_ip_addresses();

    int sockfd;
    struct sockaddr_in servaddr;
    char message[] = "DISCOVER_REQUEST";
    char buffer[1024];
    fd_set readfds;
    struct timeval tv;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    for (const auto &ip : local_ips) {
        size_t pos = ip.find_last_of('.');
        std::string base_ip = ip.substr(0, pos + 1);

        for (int i = 1; i < 255; ++i) {
            std::string test_ip = base_ip + std::to_string(i);
            inet_pton(AF_INET, test_ip.c_str(), &servaddr.sin_addr);

            for (int retry = 0; retry < max_retries; ++retry) {
                sendto(sockfd, message, strlen(message), 0,
                       (const struct sockaddr *)&servaddr, sizeof(servaddr));
            }
        }
    }

    for (int retry = 0; retry < max_retries; ++retry) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int rv = select(sockfd + 1, &readfds, nullptr, nullptr, &tv);

        if (rv == -1) {
            perror("select");
        } else if (rv == 0) {
            break;
        } else {
            if (FD_ISSET(sockfd, &readfds)) {
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);
                int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&cliaddr, &len);
                buffer[n] = '\0';
                std::string response(buffer);
                if (response == "DISCOVER_RESPONSE") {
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(cliaddr.sin_addr), ip, INET_ADDRSTRLEN);
                    discovered_ips.push_back(ip);
                }
            }
        }
    }

    close(sockfd);
    return discovered_ips;
}
