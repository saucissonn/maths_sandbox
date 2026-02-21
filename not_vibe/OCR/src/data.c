#include "data.h"
#include "globals.h"
#include "sdl_img.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

void free_matrix(double **matrix, int N) {
    for(int i = 0; i < N; i++)
        free(matrix[i]);
    free(matrix);
}

void print_matrix(double **input, int width, int height) {
    double v = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            v = input[y][x] * 255.0;
            if (v < 10) {
                printf("%.1lf  ", v);
            }
            else {
                if (v < 100) {
                    printf("%.1lf ", v);
                }
                else {
                    printf("%.1lf", v);
                }
            }
        }
        printf("\n");
    }
}

int select_random_file(void) {
    random_x = rand() % nb_output_neurons;
    expected = random_x;

    char path2[256];
    snprintf(path2, sizeof(path2), "../src/training/%d", expected);

    DIR *dir = opendir(path2);
    if (!dir) return 1;

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL)
        if (entry->d_type == DT_REG) count++;

    if (count == 0) { closedir(dir); return 1; }

    int target = rand() % count;
    rewinddir(dir);

    int i = 0;
    entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (i == target) break;
            i++;
        }
    }
    closedir(dir);
    if (!entry) return 1;

    char path[256];
    snprintf(path, sizeof(path), "../src/training/%d/%s", expected, entry->d_name);

    if (create_window(path) != 0) return 1;
    /*
    if (rgba->w != width || rgba->h != height) {
        printf("SIZE MISMATCH: got %dx%d expected %dx%d\n", rgba->w, rgba->h, width, height);
        return 1;
    }
    */

    return 0;
}
