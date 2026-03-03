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

    char line[1024];

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

static token *make_token_from_id(int id, int pos) {
    token *t = malloc(sizeof(token));
    if (!t) return NULL;
    t->id = id;
    t->content = vocab_tokens[id];
    t->embedding = malloc(dmodel * sizeof(double *));
    for (int i = 0; i < dmodel; i++) {
        (t->embedding)[i] = vocab_embed[id][i] + pos_embed[pos][i];
    }
    return t;
}

static int match_longest_token(const char *p, int *out_len) {
    int best_id = -1;
    int best_len = 0;

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

token **str_to_tokens(char *text, int *out_n) {
    if (out_n) *out_n = 0;
    if (!text || !out_n) return NULL;

    int cap = 128;
    int n = 0;
    token **arr = malloc(cap * sizeof(token *));
    if (!arr) return NULL;

    token *t = make_token_from_id(2, n); //<BOS>
    arr[n] = t;
    n += 1;

    const char *p = text;

    while (*p) {
        
        while (*p && *p == '\n') p++;
        if (!*p) break;

        int len = 0;
        int id = match_longest_token(p, &len);

        if (id < 0 || len <= 0) {
            id = 1;
            len = 1;
        }

        t = make_token_from_id(id, n);
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

    t = make_token_from_id(3, n); //<EOS>
    arr[n] = t;
    n += 1;

    token **tmp = realloc(arr, n * sizeof(token *));
    if (tmp) arr = tmp;

    *out_n = n;
    return arr;
}

void init_tokens() {
    vocab_tokens = gen_tokens_array("../src/tokens/data.txt");
    if (!vocab_tokens) {
        printf("Error vocab loading\n");
        return;
    }

    vocab_embed = malloc(sizeof(double*) * vocab_size);
    for (int i = 0; i < vocab_size; i++) {
        vocab_embed[i] = gen_random_array(vocab_size, dmodel);
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
        if (vocab_embed[i]) free(vocab_embed[i]);
        if (vocab_tokens[i]) free(vocab_tokens[i]);
    }
    if (vocab_embed) free(vocab_embed);
    if (vocab_tokens) free(vocab_tokens);
}