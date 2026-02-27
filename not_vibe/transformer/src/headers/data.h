#ifndef DATA_H
#define DATA_H

#include "nn.h"

void free_matrix(double **matrix, int N);
void print_matrix(double **input, int width, int height);
int select_random_file(void);
int layer_save(FILE *f, const struct layer *l);
int layer_load(FILE *f, struct layer *l);

#endif
