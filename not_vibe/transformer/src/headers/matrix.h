#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

double **matrix_mul(double **A, double **B, int n, int m, int p);
double **matrix_mul_transpose(double **A, double **B, int n, int m, int p);
double **matrix_mul_sub(
    double **A, int ai_start, int ai_end, int aj_start, int aj_end,
    double **B, int bi_start, int bi_end, int bj_start, int bj_end,
    int causal_mask
);

double **matrix_sum(double **m1, double **m2, int h, int w);
double mean_row(double *m, int s);
double variance_row(double *m, int s, double mean);
void free_matrix(double **m, int H);

#endif
