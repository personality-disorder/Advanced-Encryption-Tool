#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include "file_manager.h"
#include "encryption.h"
#include "utils.h"

#define MAX_PATH 1024
#define MAX_FILES 1024
#define AES_256_KEY_SIZE 32

void print_menu() {
    mvprintw(LINES - 2, 0, "Используйте стрелки для навигации, ENTER для выбора, Q для выхода.");
}

void handle_signal(int sig) {
    endwin();
    exit(0);
}

// Функция для безопасного ввода пароля с маскировкой
void get_password(char *password, size_t size) {
    noecho();
    int ch, i = 0;
    while ((ch = getch()) != '\n' && ch != '\r' && i < size - 1) {
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (i > 0) {
                i--;
                mvwdelch(stdscr, getcury(stdscr), getcurx(stdscr) - 1);
            }
        } else {
            password[i++] = ch;
            addch('*');
        }
    }
    password[i] = '\0';
    echo();
}

void navigate_to_parent_directory(char *current_path) {
    if (strcmp(current_path, "/") == 0) {
        // Уже в корневом каталоге
        return;
    }
    char temp_path[MAX_PATH];
    strncpy(temp_path, current_path, sizeof(temp_path));
    temp_path[sizeof(temp_path) - 1] = '\0';
    char *parent = dirname(temp_path);
    if (parent != NULL) {
        strncpy(current_path, parent, MAX_PATH);
        current_path[MAX_PATH - 1] = '\0';
    }
}

int main() {
    char current_path[MAX_PATH] = ".";
    char key[AES_256_KEY_SIZE];
    FileItem items[MAX_FILES];
    int item_count = 0;
    int selected = 0;

    // Обработка сигналов для корректного выхода
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // Выбранный элемент
    init_pair(2, COLOR_CYAN, -1);          // Каталоги
    init_pair(3, COLOR_YELLOW, -1);        // Файлы
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    while (1) {
        clear();
        // Отображение текущего пути
        mvprintw(0, 0, "Текущий путь: %s", current_path);
        // Список файлов и каталогов
        if (list_files(current_path, items, &item_count) != 0) {
            mvprintw(1, 0, "Ошибка при чтении каталога.");
            refresh();
            getch();
            continue;
        }
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
            case 10: // Клавиша Enter
                if (!items[selected].is_dir) {
                    clear();
                    mvprintw(0, 0, "Выбранный файл: %s", items[selected].name);
                    mvprintw(2, 0, "Выберите действие:");
                    mvprintw(3, 0, "1. Зашифровать AES-256");
                    mvprintw(4, 0, "2. Расшифровать AES-256");
                    mvprintw(5, 0, "3. Зашифровать ChaCha20");
                    mvprintw(6, 0, "4. Расшифровать ChaCha20");
                    mvprintw(7, 0, "5. Отмена");
                    refresh();

                    int action = getch() - '0';
                    if (action >= 1 && action <= 4) {
                        mvprintw(9, 0, "Введите пароль: ");
                        get_password(key, AES_256_KEY_SIZE);

                        char full_path[MAX_PATH];
                        int result = snprintf(full_path, sizeof(full_path), "%s/%s", current_path, items[selected].name);
                        if (result < 0 || (size_t)result >= sizeof(full_path)) {
                            mvprintw(10, 0, "Слишком длинное имя пути");
                            refresh();
                            getch();
                            break;
                        }

                        char output_file[MAX_PATH];
                        const char *ext = (action % 2 == 0) ? "dec" : "enc";
                        result = snprintf(output_file, sizeof(output_file), "%s.%s", full_path, ext);
                        if (result < 0 || (size_t)result >= sizeof(output_file)) {
                            mvprintw(10, 0, "Слишком длинное имя выходного файла");
                            refresh();
                            getch();
                            break;
                        }

                        int status = 0;
                        switch (action) {
                            case 1:
                                status = encrypt_aes256(full_path, output_file, key);
                                break;
                            case 2:
                                status = decrypt_aes256(full_path, output_file, key);
                                break;
                            case 3:
                                status = encrypt_chacha20(full_path, output_file, key);
                                break;
                            case 4:
                                status = decrypt_chacha20(full_path, output_file, key);
                                break;
                        }

                        // Очистка ключа из памяти
                        memset(key, 0, sizeof(key));

                        if (status != 0) {
                            mvprintw(11, 0, "Ошибка при обработке файла.");
                        } else {
                            mvprintw(11, 0, "Операция успешно завершена.");
                        }
                        refresh();
                        getch();
                    }
                } else {
                    if (strcmp(items[selected].name, "..") == 0) {
                        navigate_to_parent_directory(current_path);
                    } else {
                        char new_path[MAX_PATH];
                        int result = snprintf(new_path, sizeof(new_path), "%s/%s", current_path, items[selected].name);
                        if (result < 0 || (size_t)result >= sizeof(new_path)) {
                            mvprintw(10, 0, "Слишком длинный новый путь");
                            refresh();
                            getch();
                        } else {
                            strncpy(current_path, new_path, sizeof(current_path));
                            current_path[sizeof(current_path) - 1] = '\0'; // Гарантируем завершение строки
                        }
                    }
                    selected = 0;
                }
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
