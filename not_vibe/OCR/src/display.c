#include <SDL2/SDL.h>
#include <stdio.h>

#include "headers/display.h"

void blit_black_white(SDL_Window *win, SDL_Surface *src, int thr) {
    if (!src) return;
    SDL_Surface *screen = SDL_GetWindowSurface(win);
    if (!screen) return;

    SDL_Surface *bw = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!bw) { printf("CreateRGBSurface error: %s\n", SDL_GetError()); return; }

    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(bw))  SDL_LockSurface(bw);

    Uint8 *sp = (Uint8*)src->pixels;
    Uint8 *dp = (Uint8*)bw->pixels;
    int spitch = src->pitch;
    int dpitch = bw->pitch;

    for (int y = 0; y < src->h; y++) {
        Uint8 *srow = sp + y * spitch;
        Uint8 *drow = dp + y * dpitch;
        for (int x = 0; x < src->w; x++) {
            Uint8 r = srow[x*4 + 0];
            Uint8 g = srow[x*4 + 1];
            Uint8 b = srow[x*4 + 2];
            int lum = (r + g + b) / 3;
            Uint8 v = (lum >= thr) ? 255 : 0;
            drow[x*4 + 0] = v;
            drow[x*4 + 1] = v;
            drow[x*4 + 2] = v;
            drow[x*4 + 3] = 255;
        }
    }

    if (SDL_MUSTLOCK(bw))  SDL_UnlockSurface(bw);
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);

    SDL_BlitSurface(bw, NULL, screen, NULL);
    SDL_UpdateWindowSurface(win);
    SDL_FreeSurface(bw);
}