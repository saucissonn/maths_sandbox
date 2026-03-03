#include "headers/globals.h"
#include "headers/nn.h"

int width = 1000;
int height = 1000;
int top = 0;

int INIT_CAP_STACK = 64;

int expected = 0;
double loss = 0.0;
char *raw_data = NULL;

int size_file = 99;
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

struct layer input_layer_encoder;
struct layer hidden_layer_encoder;
struct layer output_layer_encoder;
struct layer input_layer_decoder;
struct layer hidden_layer_decoder;
struct layer output_layer_decoder;

int vocab_size = 7972;
int dmodel = 512;

char **vocab_tokens = NULL;
double **vocab_embed = NULL;
double **pos_embed = NULL;

double **input_matrix_encoder = NULL;
double **input_matrix_decoder = NULL;
double **W_last = NULL;
double *b_last = NULL;
