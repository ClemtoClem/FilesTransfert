#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <vector>
#include <string>

std::vector<std::string> get_local_ip_addresses();
std::vector<std::string> discover_instances(int port, int max_retries);

#endif // NETWORK_UTILS_H
