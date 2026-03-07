#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/fs.h"
#include "headers/globals.h"
#include "headers/token.h"

int select_random_file_and_answer(int *seq_encoder, int *seq_decoder) {
    char folder[256];
    snprintf(folder, sizeof(folder), "../src/tokens/texts");

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
    //printf("file_inputs=%s\n", path);

    char path2[512];
    snprintf(path2, sizeof(path2), "%s_fr/%s", folder, entry->d_name);
    //printf("file_ans=%s\n", path2);
    file_inputs = fopen(path, "r");

    int t = 0;
    int t2 = 0;
    size_t len = 0;
    input = NULL;
    while (t <= random_x) {
        if ((t2 = getline(&input, &len, file_inputs)) > -1) {
            t++;
        }
    }
    input[t2-1] = '\0';
    fclose(file_inputs);

    file_ans = fopen(path2, "r");
    t = 0;
    len = 0;
    answer = NULL;
    while (t <= random_x) {
        if ((t2 = getline(&answer, &len, file_ans)) > -1) {
            t++;
        }
    }
    answer[t2-1] = '\0';
    fclose(file_ans);

    token **toks_in = str_to_tokens(input, seq_encoder, 'a');

    if (!toks_in) {
        printf("Tokenisation failed\n");
        return 1;
    }
    
    /*
    for (int i = 0; i < *seq_encoder; i++) {
        printf("Token %d: ", i+1);
        printf("content = %s", toks_in[i]->content);
        printf("\tid = %d\n", toks_in[i]->id);
    }
    */

    input_matrix_encoder = malloc(*seq_encoder * sizeof(double *));
    for (int i = 0; i < *seq_encoder; i++) {
        input_matrix_encoder[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            input_matrix_encoder[i][j] = (toks_in[i]->embedding)[j];
        }
    }

    token **toks_out = str_to_tokens(answer, seq_decoder, 'i');

    if (!toks_out) {
        printf("Tokenisation failed for answer\n");
        return 1;
    }
    /*
    for (int i = 0; i < *seq_decoder; i++) {
        printf("Token %d: ", i+1);
        printf("content = %s", toks_out[i]->content);
        printf("\tid = %d\n", toks_out[i]->id);
    }
    */

    expected_matrix = malloc(*seq_decoder * sizeof(int));
    for (int i = 0; i < *seq_decoder; i++) {
        expected_matrix[i] = toks_out[i]->id;
    }

    input_matrix_decoder = malloc(*seq_decoder * sizeof(double *));

    for (int i = 0; i < *seq_decoder; i++) {
        input_matrix_decoder[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            input_matrix_decoder[i][j] = (toks_out[i]->embedding)[j];
        }
    }

    free_tokens(toks_in, *seq_encoder);
    free_tokens(toks_out, *seq_decoder);
    
    return 0;
}
