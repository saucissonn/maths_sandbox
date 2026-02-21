#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>
#include <dirent.h>

extern SDL_Surface *img;
extern SDL_Window *win;
extern SDL_Surface *rgba;

extern int width;
extern int height;

extern int expected;
extern double loss;
extern double **raw_data;

extern int random_x;
extern DIR *dir;

extern int c_steps;

extern double learning_coeff;
extern int nb_hidden_neurons;
extern int nb_output_neurons;

#endif
