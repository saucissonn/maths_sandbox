#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/fs.h"
#include "headers/globals.h"

int select_random_file_and_answer() {
    char folder[256];
    snprintf(folder, sizeof(folder), "../src/texts");

    DIR *d = opendir(folder);
    if (!d) { printf("opendir failed: %s\n", folder); return 1; }

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_REG) count++;
    }
    if (count == 0) { closedir(d); return 1; }

    int target = rand() % count; //random file selected
    random_x = rand() % size_file; //random line
    printf("%d\n", random_x);

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
    printf("file_inputs=%s\n", path);

    char path2[512];
    snprintf(path2, sizeof(path2), "%s_ans/%s", folder, entry->d_name);
    printf("file_ans=%s\n", path2);
    file_inputs = fopen(path, "r");

    int t = 0;
    int t2 = 0;
    int *len = NULL;
    while (t <= random_x) {
        if ((t2 = getline(&input, &len, file_inputs)) > -1) {
            t++;
        }
    }
    input[t2-1] = '\0';
    fclose(file_inputs);

    file_ans = fopen(path2, "r");
    t = 0;
    while (t <= random_x) {
        if ((t2 = getline(&answer, &len, file_ans)) > -1) {
            t++;
        }
    }
    answer[t2-1] = '\0';
    fclose(file_ans);

    return 0;
}
