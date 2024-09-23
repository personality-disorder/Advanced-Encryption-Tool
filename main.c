#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include "file_manager.h"
#include "encryption.h"
#include "utils.h"

void print_menu() {
    mvprintw(LINES - 2, 0, "Use arrow keys to navigate, ENTER to select, Q to quit.");
}

int main() {
    char current_path[MAX_PATH] = ".";
    char key[AES_256_KEY_SIZE];
    FileItem items[MAX_FILES];
    int item_count = 0;
    int selected = 0;

    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_RED, -1);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    while (1) {
        clear();
        list_files(current_path, items, &item_count);
        draw_file_list(stdscr, items, item_count, selected, current_path);
        print_menu();
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected > 0) ? selected - 1 : selected;
                break;
            case KEY_DOWN:
                selected = (selected < item_count - 1) ? selected + 1 : selected;
                break;
            case 10: // Enter key
                if (!items[selected].is_dir) {
                    clear();
                    mvprintw(0, 0, "Selected file: %s", items[selected].name);
                    mvprintw(2, 0, "Choose action:");
                    mvprintw(3, 0, "1. Encrypt with AES-256");
                    mvprintw(4, 0, "2. Decrypt with AES-256");
                    mvprintw(5, 0, "3. Encrypt with ChaCha20");
                    mvprintw(6, 0, "4. Decrypt with ChaCha20");
                    mvprintw(7, 0, "5. Cancel");
                    refresh();

                    int action = getch() - '0';
                    if (action >= 1 && action <= 4) {
                        echo();
                        mvprintw(9, 0, "Enter password: ");
                        getnstr(key, AES_256_KEY_SIZE);
                        noecho();

                        char full_path[MAX_PATH];
                        int result = snprintf(full_path, sizeof(full_path), "%s/%s", current_path, items[selected].name);
                        if (result < 0 || (size_t)result >= sizeof(full_path)) {
                            fprintf(stderr, "Path name too long\n");
                            break;
                        }

                        char output_file[MAX_PATH];
                        result = snprintf(output_file, sizeof(output_file), "%s.%s", full_path, (action % 2 == 0) ? "dec" : "enc");
                        if (result < 0 || (size_t)result >= sizeof(output_file)) {
                            fprintf(stderr, "Output file name too long\n");
                            break;
                        }

                        switch (action) {
                            case 1:
                                encrypt_aes256(full_path, output_file, key);
                                break;
                            case 2:
                                decrypt_aes256(full_path, output_file, key);
                                break;
                            case 3:
                                encrypt_chacha20(full_path, output_file, key);
                                break;
                            case 4:
                                decrypt_chacha20(full_path, output_file, key);
                                break;
                        }
                    }
                } else if (strcmp(items[selected].name, "..") == 0) {
                    char *last_slash = strrchr(current_path, '/');
                    if (last_slash != NULL) {
                        *last_slash = '\0'; // Move up a directory level
                    }
                } else {
                    char new_path[MAX_PATH];
                    int result = snprintf(new_path, sizeof(new_path), "%s/%s", current_path, items[selected].name);
                    if (result < 0 || (size_t)result >= sizeof(new_path)) {
                        fprintf(stderr, "New path too long\n");
                    } else {
                        strncpy(current_path, new_path, sizeof(current_path));
                        current_path[sizeof(current_path) - 1] = '\0'; // Ensure null-termination
                    }
                }
                selected = 0;
                break;
            case 'q':
            case 'Q':
                goto cleanup;
        }
    }

cleanup:
    endwin();
    return 0;
}
