#include "FileTransfer.hpp"

void FileTransfer::send_file(const std::string &filename, int sock, std::function<void(const std::string&)> log_callback) {
    std::ifstream infile(filename, std::ios::binary);

    if (!infile.is_open()) {
        log_callback("Erreur lors de l'ouverture du fichier pour lire.");
        return;
    }

    // Send filename first
    size_t filename_length = filename.size();
    send(sock, &filename_length, sizeof(filename_length), 0);
    send(sock, filename.c_str(), filename_length, 0);

    // Send file content
    char buffer[1024] = {0};
    while (infile.read(buffer, sizeof(buffer))) {
        send(sock, buffer, infile.gcount(), 0);
    }
    send(sock, buffer, infile.gcount(), 0);

    infile.close();
    log_callback("Fichier envoye avec succes.");
}

void FileTransfer::receive_file(int sock, std::function<void(const std::string&)> log_callback) {
    // Receive filename first
    size_t filename_length;
    read(sock, &filename_length, sizeof(filename_length));
    char filename[1024] = {0};
    read(sock, filename, filename_length);
    std::string output_filename(filename, filename_length);

    std::ofstream outfile(output_filename, std::ios::binary);

    if (!outfile.is_open()) {
        log_callback("Erreur lors de l'ouverture du fichier pour ecrire.");
        close(sock);
        return;
    }

    // Receive file content
    char buffer[1024] = {0};
    int valread;
    while ((valread = read(sock, buffer, 1024)) > 0) {
        outfile.write(buffer, valread);
    }

    outfile.close();
    log_callback("Fichier '" + output_filename + "' reçu avec succes.");
}

void FileTransfer::send_stop_signal(int sock) {
    std::string stop_signal = "STOP";
    send(sock, stop_signal.c_str(), stop_signal.size(), 0);
}
