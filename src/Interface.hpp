#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <vector>
#include <string>
#include <ncurses.h>

class Interface {
public:
    Interface();
    ~Interface();

    void display_received_files(const std::vector<std::string> &files);
    void display_status(const std::string &status);
    std::vector<std::string> select_computers(const std::vector<std::string> &computers);
    std::string select_file_to_send();
    std::string navigate_filesystem();

    WINDOW* get_menu_win() const;

private:
    WINDOW *recv_win;
    WINDOW *status_win;
    WINDOW *menu_win;
    void init_windows();
    void cleanup();
};

#endif // INTERFACE_HPP
