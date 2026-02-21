#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "headers/image.h"

/*
SDL_Surface *create_rgba(const char *path) {
    SDL_Surface *loaded = IMG_Load(path);
    if (!loaded) {
        printf("IMG_Load failed: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Surface *conv = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    if (!conv) {
        printf("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return NULL;
    }
    return conv;
}
*/

int load_image(SDL_Surface **rgba, int *width, int *height, const char *path) {
    if (*rgba) {
        SDL_FreeSurface(*rgba);
        *rgba = NULL;
    }

    *rgba = create_rgba(path);
    if (!*rgba) return 1;

    *width = (*rgba)->w;
    *height = (*rgba)->h;
    return 0;
}