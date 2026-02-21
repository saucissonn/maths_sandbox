#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/image.h"
#include "headers/fs.h"

int select_random_file_and_load(SDL_Surface **rgba, int *width, int *height) {
    char folder[256];
    snprintf(folder, sizeof(folder), "../src/moby_dick");

    DIR *d = opendir(folder);
    if (!d) { printf("opendir failed: %s\n", folder); return 1; }

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_REG) count++;
    }
    if (count == 0) { closedir(d); return 1; }

    int target = rand() % count;
    rewinddir(d);

    int i = 0;
    entry = NULL;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (i == target) break;
            i++;
        }
    }
    closedir(d);
    if (!entry) return 1;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", folder, entry->d_name);
    printf("file=%s\n", path);

    return load_image(rgba, width, height, path);
}