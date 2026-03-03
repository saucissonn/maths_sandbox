#ifndef TOKEN_H
#define TOKEN_H

#include "globals.h"

typedef struct token {
    int id;
    char *content;
    double *embedding;
} token;

double rand_uniform_token(double a, double b);
double *gen_random_array(int v_size, int size);
double potential_encoding(int pos, int i);
double *gen_pos_array(int pos, int size);
int *get_ids_array(char *filename);
char **gen_tokens_array(char *filename);
token **str_to_tokens(char *text, int *out_n);
void init_tokens();
void free_tokens(token **toks, int n);
void free_vocab();

#endif
