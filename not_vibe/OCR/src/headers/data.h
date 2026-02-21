#ifndef DATA_H
#define DATA_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include "nn.h" 

double **convert_surface_to_double(SDL_Surface * input);
void free_matrix(double **matrix, int N);
void print_matrix(double **input, int width, int height);
int select_random_file(void);
int layer_save(FILE *f, const struct layer *l);
int layer_load(FILE *f, struct layer *l);

#endif
