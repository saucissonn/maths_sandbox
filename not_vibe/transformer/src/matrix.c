#include <stdlib.h>

#include "headers/matrix.h"

void free_matrix(double **m, int H) { //just free a matrix of height H
    if (!m) return;
    for (int r = 0; r < H; r++) {
        if (m[r]) {
            free(m[r]);
        }
    }
    free(m);
}
