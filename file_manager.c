#include "file_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "utils.h"

void list_files(const char *path, FileItem *items, int *item_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH];

    *item_count = 0;

    strcpy(items[*item_count].name, "..");
    items[*item_count].is_dir = 1;
    (*item_count)++;

    if ((dir = opendir(path)) == NULL) {
        perror("Cannot open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL && *item_count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &file_stat) == 0) {
            strcpy(items[*item_count].name, entry->d_name);
            items[*item_count].is_dir = S_ISDIR(file_stat.st_mode);
            (*item_count)++;
        }
    }
    closedir(dir);
}

void draw_file_list(WINDOW *win, FileItem *items, int item_count, int selected, const char *current_path) {
    int row, col;
    getmaxyx(win, row, col);
    (void)col; // Подавить предупреждение о неиспользуемой переменной

    attron(COLOR_PAIR(2));
    mvprintw(0, 0, "Current directory: %s", current_path);
    attroff(COLOR_PAIR(2));
    mvprintw(1, 0, "Press 'Q' to quit, Enter to encrypt/decrypt");

    for (int i = 0; i < item_count && i < row - 3; i++) {
        if (i == selected) {
            attron(COLOR_PAIR(1) | A_REVERSE);
        }
        mvprintw(i + 3, 0, "%d. [%c] %s", i + 1, items[i].is_dir ? 'D' : 'F', items[i].name);
        if (i == selected) {
            attroff(COLOR_PAIR(1) | A_REVERSE);
        }
    }
}
