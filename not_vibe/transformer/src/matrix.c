#include <stdlib.h>

#include "headers/matrix.h"
#include "headers/globals.h"
#include "headers/token.h"

double **matrix_mul(double **A, double **B, int n, int m, int p) {
    double **C = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        C[i] = malloc(p * sizeof(double));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < p; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < m; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

double **matrix_mul_transpose(double **A, double **B, int n, int m, int p) {
    double **C = malloc(p * sizeof(double *));
    for (int i = 0; i < p; i++) {
        C[i] = malloc(n * sizeof(double));
    }

    for (int i = 0; i < p; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < m; k++) {
                C[i][j] += A[j][k] * B[k][i];
            }
        }
    }
    return C;
}

double **matrix_mul_sub(
    double **A, int ai_start, int ai_end, int aj_start, int aj_end,
    double **B, int bi_start, int bi_end, int bj_start, int bj_end,
    int causal_mask
) {
    int n = ai_end - ai_start; // rows out
    int m = aj_end - aj_start; // dim in common
    int p = bj_end - bj_start; // cols out

    if (m != (bi_end - bi_start)) return NULL;

    double scale = 1.0 / sqrt((double)m);

    double **C = malloc((size_t)n * sizeof(double *));
    if (!C) return NULL;

    for (int i = 0; i < n; i++) {
        C[i] = malloc((size_t)p * sizeof(double));
        if (!C[i]) {
            for (int t = 0; t < i; t++) free(C[t]);
            free(C);
            return NULL;
        }
    }

    for (int i = 0; i < n; i++) {
        int q_pos = ai_start + i; // position of query
        for (int j = 0; j < p; j++) {
            int k_pos = bj_start + j; // position of key

            if (causal_mask && k_pos > q_pos) {
                C[i][j] = -INFINITY; // mask before softmax
                continue;
            }

            double sum = 0.0;
            for (int k = 0; k < m; k++) {
                sum += A[ai_start + i][aj_start + k] * B[bi_start + k][bj_start + j];
            }
            C[i][j] = sum * scale; // 1/sqrt(dk)
        }
    }
    return C;
}

double **matrix_sum(double **m1, double **m2, int h, int w) {
    double **y = malloc(h * sizeof(double *));
    for (int i = 0; i < h; i++) {
        y[i] = malloc(w * sizeof(double));
        for (int j = 0; j < w; j++) {
            y[i][j] = m1[i][j] + m2[i][j];
        }
    }
    return y;
}

double mean_row(double *m, int s) {
    double sum = 0.0;
    for (int i = 0; i < s; i++) {
        sum += m[i];
    }
    double res = sum / (double)s;
    return res;
}

double variance_row(double *m, int s, double mean) {
    double sum = 0.0;
    for (int i = 0; i < s; i++) {
        sum += (m[i] - mean)*(m[i] - mean);
    }
    double res = sum / (double)s;
    return res;
}

void free_matrix(double **m, int H) { //just free a matrix of height H
    if (!m) return;
    for (int r = 0; r < H; r++) {
        if (m[r]) {
            free(m[r]);
        }
    }
    free(m);
}
