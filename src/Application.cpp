#include "Application.hpp"
#include "NetworkUtils.hpp"

#define PORT 8080

Application::Application() : running(true) {}

void Application::run() {
    std::thread server_thread(&Application::server_mode, this);

    std::vector<std::string> discovered_computers;
    std::vector<std::string> selected_computers;
    while (running) {
        discovered_computers = search_for_computers();
        if (discovered_computers.empty()) {
            ui.display_status("Aucun ordinateur trouvée. Appuyez sur Entree pour relancer la recherche.");
            int ch;
            while ((ch = wgetch(ui.get_menu_win())) != ERR) {
                if (ch == 10) {  // Entree
                    continue;  // Relancer la recherche
                } else if (ch == 27) {  // Echap
                    running = false;  // Quitter l'application
                    break;
                }
            }
            continue;
        }
  
        selected_computers = ui.select_computers(discovered_computers);
        if (selected_computers.empty()) {
            ui.display_status("Aucun ordinateur selectionne. Appuyez sur Entree pour relancer la selection.");
            int ch;
            while ((ch = wgetch(ui.get_menu_win())) != ERR) {
                if (ch == 10) {  // Entree
                    continue;  // Relancer la recherche
                } else if (ch == 27) {  // Echap
                    running = false;  // Quitter l'application
                    break;
                }
            }
            continue;
        }

        std::string file_to_send = ui.select_file_to_send();
        ui.display_status("Envoi du fichier: " + file_to_send);

        for (const auto &computer : selected_computers) {
            std::thread client_thread(&Application::client_mode, this, computer, file_to_send);
            client_thread.detach();
        }

        // Monitor for ESC key
        int ch;
        while ((ch = wgetch(ui.get_menu_win())) != ERR) {
            if (ch == 27) {  // ESC key
                running = false;
                ui.display_status("Arrêt en cours...");

                // Send stop signal to selected computers
                for (const auto &computer : selected_computers) {
                    int sock = 0;
                    struct sockaddr_in serv_addr;
                    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("Socket creation error");
                        continue;
                    }

                    serv_addr.sin_family = AF_INET;
                    serv_addr.sin_port = htons(PORT);

                    if (inet_pton(AF_INET, computer.c_str(), &serv_addr.sin_addr) <= 0) {
                        perror("Invalid address / Address not supported");
                        continue;
                    }

                    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                        perror("Connection Failed");
                        continue;
                    }

                    FileTransfer::send_stop_signal(sock);
                    close(sock);
                }

                break;
            }
        }
    }

    server_thread.join();
}

std::vector<std::string> Application::search_for_computers() {
    ui.display_status("Recherche d'ordinateurs en cours...");

    int max_retries = 5;
    std::vector<std::string> discovered_computers = discover_instances(PORT, max_retries);
    
    if (discovered_computers.empty()) {
        ui.display_status("Aucun ordinateur trouve. Appuyez sur Entree pour relancer la recherche.");
        int ch;
        while ((ch = wgetch(ui.get_menu_win())) != ERR) {
            if (ch == 10) {  // Entree
                return search_for_computers();  // Relancer la recherche
            }
        }
    }

    return discovered_computers;
}

void Application::server_mode()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (running) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char buffer[1024] = {0};
        int valread = read(new_socket, buffer, sizeof(buffer));
        if (valread > 0) {
            std::string received_msg(buffer, valread);
            if (received_msg == "STOP") {
                ui.display_status("Arrêt de la reception des fichiers.");
                close(new_socket);
                continue;
            }
        }

        std::thread receive_thread(&FileTransfer::receive_file, new_socket, [this](const std::string &msg) {
            ui.display_status(msg);
        });
        receive_thread.detach();
    }

    close(server_fd);
}

void Application::client_mode(const std::string &address, const std::string &file) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return;
    }

    FileTransfer::send_file(file, sock, [this](const std::string &msg) {
        ui.display_status(msg);
    });
    close(sock);
}
