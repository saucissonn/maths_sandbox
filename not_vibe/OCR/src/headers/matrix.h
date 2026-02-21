#ifndef MATRIX_H
#define MATRIX_H

#include <SDL2/SDL.h>

void free_matrix(double **m, int H);
double **surface_to_binary_matrix(SDL_Surface *input, int top_crop, int thr, int *outH, int *outW);

#endif