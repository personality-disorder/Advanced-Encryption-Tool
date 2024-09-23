#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <ncurses.h>
#include "utils.h"

#define MAX_FILES 1000

typedef struct {
    char name[256];
    int is_dir;
} FileItem;

void list_files(const char *path, FileItem *items, int *item_count);
void draw_file_list(WINDOW *win, FileItem *items, int item_count, int selected, const char *current_path);

#endif // FILE_MANAGER_H
