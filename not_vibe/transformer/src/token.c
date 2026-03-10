#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "headers/globals.h"
#include "headers/data.h"
#include "headers/nn.h"
#include "headers/matrix.h"
#include "headers/token.h"

double rand_uniform_token(double a, double b) {
    return a + (b - a) * (rand() / (double)RAND_MAX);
}

double *gen_random_array(int v_size, int size) { //v_size is vocab_size, size is dmodel. 
    double *E = malloc(sizeof(double) * size);

    double limit = sqrtf(6.0 / (v_size + size));

    for (int i = 0; i < size; i++) {
        E[i] = rand_uniform_token(-limit, limit);
    }
    return E;
}

double potential_encoding(int pos, int i) {
    double value = (pos / pow(10000, 2*i/(double)dmodel));
    if (i % 2) {
        return cos(value);
    }
    return sin(value);
}

double *gen_pos_array(int pos, int size) {
    double *E = malloc(sizeof(double) * size);

    for (int i = 0; i < size; i++) {
        E[i] = potential_encoding(pos, i);
    }
    return E;
}

char **gen_tokens_array(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    char **tokens = malloc(vocab_size * sizeof(char *));
    int size = 0;

    char line[10000];

    while (fgets(line, sizeof(line), file)) {
        char *p = strchr(line, ':');
        if (!p) continue;

        p += 2; // skip : and space

        char *end = strchr(p, '\n');
        if (end) *end = '\0';

        tokens[size++] = strdup(p);
    }

    fclose(file);
    return tokens;
}

static token *make_token_from_id(int id, int pos, char mode) {
    token *t = malloc(sizeof(token));
    if (!t) return NULL;
    t->id = id;
    char **vocab_tokens;
    if (mode == 'i') {
        vocab_tokens = vocab_tokens_input;
    }
    else {
        vocab_tokens = vocab_tokens_answer;
    }
    t->content = vocab_tokens[id];
    t->embedding = malloc(dmodel * sizeof(double *));
    if (mode == 'i') { //input
        for (int i = 0; i < dmodel; i++) {
            (t->embedding)[i] = vocab_embed_input[id][i] * Wvocab_embed_input[id][i] + pos_embed[pos][i];
        }
    }
    if (mode == 'a') { //answer
        for (int i = 0; i < dmodel; i++) {
            (t->embedding)[i] = vocab_embed_answer[id][i] * Wvocab_embed_answer[id][i] + pos_embed[pos][i];
        }
    }
    return t;
}

static int match_longest_token(const char *p, int *out_len, char mode) {
    int best_id = -1;
    int best_len = 0;
    char **vocab_tokens;
    if (mode == 'i') {
        vocab_tokens = vocab_tokens_input;
    }
    else {
        vocab_tokens = vocab_tokens_answer;
    }

    for (int i = 0; i < vocab_size; i++) {
        const char *tok = vocab_tokens[i];
        if (!tok) continue;

        int len = strlen(tok);
        if (len <= best_len) continue;

        if (strncmp(p, tok, len) == 0) {
            best_id = i;
            best_len = len;
        }
    }

    if (best_id >= 0) {
        *out_len = best_len;
        return best_id;
    }
    *out_len = 0;
    return -1;
}

token **str_to_tokens(char *text, int *out_n, char mode) {
    if (out_n) *out_n = 0;
    if (!text || !out_n) return NULL;

    int cap = 128;
    int n = 0;
    token **arr = malloc(cap * sizeof(token *));
    if (!arr) return NULL;

    token *t = make_token_from_id(2, n, mode); //<BOS>
    arr[n] = t;
    n += 1;

    const char *p = text;

    while (*p) {
        
        while (*p && *p == '\n') p++;
        if (!*p) break;

        int len = 0;
        int id = match_longest_token(p, &len, mode);

        if (id < 0 || len <= 0) {
            id = 1;
            len = 1;
        }

        t = make_token_from_id(id, n, mode);
        if (!t) {
            for (int i = 0; i < n; i++) free(arr[i]);
            free(arr);
            return NULL;
        }

        if (n >= cap) {
            cap *= 2;
            token **tmp = realloc(arr, cap * sizeof(token *));
            if (!tmp) {
                free(t);
                for (int i = 0; i < n; i++) free(arr[i]);
                free(arr);
                return NULL;
            }
            arr = tmp;
        }

        arr[n++] = t;
        p += len;
    }

    t = make_token_from_id(3, n, mode); //<EOS>
    arr[n] = t;
    n += 1;

    token **tmp = realloc(arr, n * sizeof(token *));
    if (tmp) arr = tmp;

    *out_n = n;
    return arr;
}

void init_tokens() {
    vocab_tokens_answer = gen_tokens_array("../src/tokens/data.txt");
    vocab_tokens_input = gen_tokens_array("../src/tokens/data_fr.txt");
    if (!vocab_tokens_answer || !vocab_tokens_input) {
        printf("Error vocab loading\n");
        return;
    }

    vocab_embed_answer = malloc(sizeof(double*) * vocab_size);
    Wvocab_embed_answer = malloc(sizeof(double*) * vocab_size);
    dWvocab_embed_answer = malloc(sizeof(double*) * vocab_size);
    vocab_embed_input = malloc(sizeof(double*) * vocab_size);
    Wvocab_embed_input = malloc(sizeof(double*) * vocab_size);
    dWvocab_embed_input = malloc(sizeof(double*) * vocab_size);
    for (int i = 0; i < vocab_size; i++) {
        vocab_embed_answer[i] = gen_random_array(vocab_size, dmodel);
        Wvocab_embed_answer[i] = gen_random_array(vocab_size, dmodel);
        dWvocab_embed_answer[i] = gen_random_array(vocab_size, dmodel);
        vocab_embed_input[i] = gen_random_array(vocab_size, dmodel);
        Wvocab_embed_input[i] = gen_random_array(vocab_size, dmodel);
        dWvocab_embed_input[i] = gen_random_array(vocab_size, dmodel);
    }

    pos_embed = malloc(sizeof(double*) * vocab_size);
    for (int i = 0; i < vocab_size; i++) {
        pos_embed[i] = gen_pos_array(i, dmodel);
    }
}

void free_tokens(token **toks, int n) {
    if (!toks) return;
    for (int i = 0; i < n; i++) {
        if (toks[i])
            free((toks[i])->embedding);
        free(toks[i]);
    }
    free(toks);
}

void free_vocab() {
    for (int i = 0; i < vocab_size; i++) {
        free(vocab_embed_answer[i]);
        free(vocab_embed_input[i]);
        free(vocab_tokens_answer[i]);
        free(vocab_tokens_input[i]);
    }
    free(vocab_embed_answer);
    free(vocab_embed_input);
    free(vocab_tokens_input);
    free(vocab_tokens_answer);
}