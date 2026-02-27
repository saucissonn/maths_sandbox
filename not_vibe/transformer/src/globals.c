#include "headers/globals.h"

int width = 1000;
int height = 1000;
int top = 0;

int INIT_CAP_STACK = 64;

int expected = 0;
double loss = 0.0;
char *raw_data = NULL;

int size_file = 2;
int random_x = 0;
char *input = NULL;
char *answer = NULL;
DIR *dir = NULL;
FILE *file_inputs;
FILE *file_ans;

int c_steps = 0;

double learning_coeff = 0.01;
int nb_hidden_neurons = 256;
int nb_output_neurons = 62;
