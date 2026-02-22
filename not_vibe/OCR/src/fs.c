#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/image.h"
#include "headers/fs.h"
#include "headers/globals.h"

int select_random_file_and_load(SDL_Surface **rgba, int *width, int *height) {
    char folder[256];
    snprintf(folder, sizeof(folder), "../src/images");

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
    printf("file_image=%s\n", path);
    char path2[512];
    int temp = strlen(entry->d_name);
    entry->d_name[temp - 1] = 't';
    entry->d_name[temp - 2] = 'x';
    entry->d_name[temp - 3] = 't';
    snprintf(path2, sizeof(path2), "%s_ans/%s", folder, entry->d_name);
    printf("file_ans=%s\n", path2);
    file_ans = fopen(path2, "r");

    return load_image(rgba, width, height, path);
}

int char_to_index(char c) {
    //normalisation des accents
    if (c == 'é' || c == 'è') c = 'e';
    if (c == 'à') c = 'a';

    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 26;

    if (c >= '0' && c <= '9') return c - '0' + 52;

    switch (c) {
        case '?': return 62;
        case '.': return 63;
        case '!': return 64;
        case '&': return 65;
        case '(': return 66;
        case '-': return 67;
        case '_': return 68;
        case ')': return 69;
        case '%': return 70;
        case ',': return 71;
        case ';': return 72;
        case '@': return 73;
    }

    return -1;
}

char index_to_char(int i) {
    // a-z
    if (i >= 0 && i <= 25)
        return 'a' + i;

    // A-Z
    if (i >= 26 && i <= 51)
        return 'A' + (i - 26);

    // 0-9
    if (i >= 52 && i <= 61)
        return '0' + (i - 52);

    // symboles
    switch (i) {
        case 62: return '?';
        case 63: return '.';
        case 64: return '!';
        case 65: return '&';
        case 66: return '(';
        case 67: return '-';
        case 68: return '_';
        case 69: return ')';
        case 70: return '%';
        case 71: return ',';
        case 72: return ';';
        case 73: return '@';
    }

    return '\0'; // invalide
}