#include <SDL2/SDL.h>
#include <stdlib.h>

#include "headers/matrix.h"

void free_matrix(double **m, int H) {
    if (!m) return;
    for (int r = 0; r < H; r++) free(m[r]);
    free(m);
}

double **surface_to_binary_matrix(SDL_Surface *input, int top_crop, int thr, int *outH, int *outW) {
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

    for (int y = top_crop; y < Hfull; y++) {
        int rr = y - top_crop;
        Uint8 *row = base + y * pitch;
        for (int x = 0; x < W; x++) {
            Uint8 r = row[x*4 + 0];
            Uint8 g = row[x*4 + 1];
            Uint8 b = row[x*4 + 2];
            int lum = (r + g + b) / 3;
            out[rr][x] = (lum < thr) ? 1.0 : 0.0;
        }
    }

    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input);

    *outH = H;
    *outW = W;
    return out;
}