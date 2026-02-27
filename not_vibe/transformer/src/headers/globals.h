#ifndef GLOBALS_H
#define GLOBALS_H

#include <dirent.h>
#include <stddef.h>
#include <stdio.h>

extern int width;
extern int height;
extern int top;

extern int expected;
extern double loss;
extern char *raw_data;

extern int size_file;
extern int random_x;
extern char *input;
extern char *answer;
extern DIR *dir;
extern FILE *file_inputs;
extern FILE *file_ans;

extern int c_steps;

extern double learning_coeff;
extern int nb_hidden_neurons;
extern int nb_output_neurons;

extern int INIT_CAP_STACK;

#endif
