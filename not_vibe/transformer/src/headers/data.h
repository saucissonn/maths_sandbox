#ifndef DATA_H
#define DATA_H

#include "nn.h"

void free_matrix(double **matrix, int N);
void print_matrix(double **input, int height, int width);
void printf_time_diff(struct timespec start, struct timespec end);
int layer_save(FILE *f, const struct layer *l);
int layer_load(FILE *f, struct layer *l);

#endif
