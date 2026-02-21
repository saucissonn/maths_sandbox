#ifndef SDL_IMG_UTILS_H
#define SDL_IMG_UTILS_H

#include <SDL2/SDL.h>

SDL_Surface * create_rgba(char * image);
int create_window(char * image);
void display_SDL_matrix(SDL_Surface * input, int width, int height);
double **convert_surface_to_double(SDL_Surface * input);

#endif
