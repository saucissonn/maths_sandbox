#include <SDL2/SDL.h>
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

double **surface_to_binary_matrix(SDL_Surface *input, int top_crop, int thr, int *outH, int *outW) {
    //convert a SDL surface (often the whole image) to a matrix with 0s and 1s (no color). from top_crop to the bottom of the window
    //outH and and outW to not rely on the input value (often named rgba)
    if (!input) return NULL;

    if (SDL_MUSTLOCK(input)) SDL_LockSurface(input);

    int W = input->w;
    int Hfull = input->h;
    int H = Hfull - top_crop;
    if (H <= 0) {
        if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input);
        return NULL;
    }

    double **out = (double**)malloc((size_t)H * sizeof(double*));
    if (!out) {
        if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input);
        return NULL;
    }
    for (int r = 0; r < H; r++) {
        out[r] = (double*)malloc((size_t)W * sizeof(double));
        if (!out[r]) {
            for (int k = 0; k < r; k++) free(out[k]);
            free(out);
            if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input);
            return NULL;
        }
    }

    Uint8 *base = (Uint8*)input->pixels;
    int pitch = input->pitch;
    int rr;
    Uint8 *row;
    Uint8 r;
    Uint8 g;
    Uint8 b;
    int lum;

    for (int y = top_crop; y < Hfull; y++) {
        rr = y - top_crop;
        row = base + y * pitch;
        for (int x = 0; x < W; x++) {
            r = row[x*4 + 0];
            g = row[x*4 + 1];
            b = row[x*4 + 2];
            lum = (r + g + b) / 3;
            out[rr][x] = (lum < thr) ? 1.0 : 0.0;
        }
    }

    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input); //have to unlock or undefined behavior

    *outH = H;
    *outW = W;
    return out;
}
