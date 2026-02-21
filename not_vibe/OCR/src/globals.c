#include "globals.h"

SDL_Surface *img = NULL;
SDL_Window *win = NULL;
SDL_Surface *rgba = NULL;

int width = 28;
int height = 28;

int expected = 0;
double loss = 0.0;
double **raw_data = NULL;

int random_x = 5;
DIR *dir = NULL;

int c_steps = 0;

double learning_coeff = 0.001;
int nb_hidden_neurons = 256;
int nb_output_neurons = 10;
