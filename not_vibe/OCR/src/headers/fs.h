#ifndef FS_H
#define FS_H

#include <SDL2/SDL.h>

int select_random_file_and_load(SDL_Surface **rgba, int *width, int *height);
int char_to_index(char c);

#endif