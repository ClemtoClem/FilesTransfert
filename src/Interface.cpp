#include "Interface.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

Interface::Interface() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    setlocale(LC_CTYPE, "");
    init_windows();
}

Interface::~Interface() {
    cleanup();
    endwin();
}

void Interface::init_windows() {
    int height = LINES - 4;
    int width = COLS / 2 - 2;
    recv_win = newwin(height, width, 1, 1);
    status_win = newwin(height, width, 1, COLS / 2 + 1);
    menu_win = newwin(3, COLS - 2, LINES - 3, 1);
    box(recv_win, 0, 0);
    box(status_win, 0, 0);
    box(menu_win, 0, 0);
    mvwprintw(recv_win, 0, 1, " Fichiers reçus ");
    mvwprintw(status_win, 0, 1, " Status ");
    mvwprintw(menu_win, 0, 1, " Menu ");
    wrefresh(recv_win);
    wrefresh(status_win);
    wrefresh(menu_win);
}

void Interface::cleanup() {
    delwin(recv_win);
    delwin(status_win);
    delwin(menu_win);
}

void Interface::display_status(const std::string &status) {
    int height = LINES - 4;
    int width = COLS / 2 - 2;
    werase(status_win);
    box(status_win, 0, 0);
    mvwprintw(status_win, 0, 1, " Status ");
    int line = status.size() / (width-4) + 1;
    for (int i = 0; i < line; ++i) {
        for (int j = 0; j < width-4; ++j) {
            if (static_cast<size_t>(i * (width-4) + j) >= status.size()) break;
            mvwprintw(status_win, i + 2, j + 2, "%c", status[i * (width-4) + j]);
        }
    }
    wrefresh(status_win);
}

void Interface::display_received_files(const std::vector<std::string> &files) {
    int height = LINES - 4;
    int width = COLS / 2 - 2;
    werase(recv_win);
    box(recv_win, 0, 0);
    mvwprintw(recv_win, 0, 1, " Fichiers reçus ");
    for (size_t k = 0; k < files.size(); ++k) {
        int line = files[k].size() / (width-4) + 1;
        for (int i = 0; i < line; ++i) {
            for (int j = 0; j < width-4; ++j) {
                if (static_cast<size_t>(i * (width-4) + j) >= files[k].size()) break;
                mvwprintw(status_win, i + 2, j + 2, "%c", files[k][i * (width-4) + j]);
            }
        }
    }
    wrefresh(recv_win);
}

std::string Interface::select_file_to_send() {
    std::string path = navigate_filesystem();
    return path;
}

std::string Interface::navigate_filesystem() {
    std::string current_path = ".";
    std::vector<std::string> files;
    std::string selected_file;
    int highlight = 0;
    int ch;

    while (true) {
        werase(menu_win);
        box(menu_win, 0, 0);
        mvwprintw(menu_win, 0, 1, " Navigation : %s", current_path.c_str());

        files.clear();
        DIR *dir = opendir(current_path.c_str());
        if (dir == nullptr) {
            display_status("Erreur d'ouverture du repertoire.");
            return "";
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            files.push_back(entry->d_name);
        }
        closedir(dir);

        for (size_t i = 0; i < files.size(); ++i) {
            if (i == static_cast<size_t>(highlight)) {
                wattron(menu_win, A_REVERSE);
            }
            mvwprintw(menu_win, i + 1, 1, "%s", files[i].c_str());
            wattroff(menu_win, A_REVERSE);
        }

        wrefresh(menu_win);
        ch = wgetch(menu_win);

        switch (ch) {
            case KEY_UP:
                highlight = (highlight == 0) ? files.size() - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (static_cast<size_t>(highlight) == files.size() - 1) ? 0 : highlight + 1;
                break;
            case 10:  // Enter key
                if (files[highlight] == "..") {
                    current_path = current_path.substr(0, current_path.find_last_of('/'));
                    if (current_path.empty()) {
                        current_path = ".";
                    }
                } else {
                    std::string selected = files[highlight];
                    struct stat st;
                    if (stat(selected.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                        current_path += "/" + selected;
                    } else {
                        selected_file = current_path + "/" + selected;
                        return selected_file;
                    }
                }
                break;
            case 'q':
                return "";  // Quit if 'q' is pressed
            default:
                break;
        }
    }
}

WINDOW* Interface::get_menu_win() const {
    return menu_win;
}

std::vector<std::string> Interface::select_computers(const std::vector<std::string> &computers) {
    std::vector<std::string> selected;
    int highlight = 0;
    int ch;
    bool done = false;

    while (!done) {
        werase(menu_win);
        box(menu_win, 0, 0);
        mvwprintw(menu_win, 0, 1, " Selection des ordinateurs ");

        for (size_t i = 0; i < computers.size(); ++i) {
            if (static_cast<size_t>(highlight) == i) {
                wattron(menu_win, A_REVERSE);  // Highlight selected item
            }
            mvwprintw(menu_win, i + 1, 1, "%s", computers[i].c_str());
            wattroff(menu_win, A_REVERSE);  // Remove highlighting
        }

        wrefresh(menu_win);
        ch = wgetch(menu_win);

        switch (ch) {
            case KEY_UP:
                highlight = (highlight == 0) ? computers.size() - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (static_cast<size_t>(highlight) == computers.size() - 1) ? 0 : highlight + 1;
                break;
            case 10:  // Enter key
                if (std::find(selected.begin(), selected.end(), computers[highlight]) == selected.end()) {
                    selected.push_back(computers[highlight]);
                }
                break;
            case ' ':
                done = true;
                break;
            default:
                break;
        }
    }

    return selected;
}