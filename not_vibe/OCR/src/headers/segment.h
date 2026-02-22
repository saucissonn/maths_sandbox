#ifndef SEGMENT_H
#define SEGMENT_H

typedef struct { int y0, y1; } Line;
typedef struct { int min_r, min_c, max_r, max_c; int area; } Box;

void segment_and_print(double **data, int H, int W);
double **segment_to_matrix_28x28(double **data, int H, int W, int *out_n);

#endif