#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>

SDL_Surface *create_rgba(const char *path);
int load_image(SDL_Surface **rgba, int *width, int *height, const char *path);

#endif