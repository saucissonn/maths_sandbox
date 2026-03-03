#ifndef NN_H
#define NN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "matrix.h"

struct layer {
    char *name;
    int previous_size;
    int current_size;  //256 then 128
    double *weights;   //matrix (output_size × input_size)
    double *biases;    //output_size
    double *output;    //activations
    double *delta;     //gradients
    double *z;
};

struct multi_head_attention_layer {
    int masked;
    double **Wq;
    double **Wk;
    double **Wv;
    double **Wo;
    double **Q;
    double **K;
    double **V;
    double *gamma;
    double *beta;
};

struct feed_forward_layer {
    struct layer input_layer;
    struct layer hidden_layer;
    struct layer output_layer;
    double *gamma;
    double *beta;
};

void relu(struct layer *l);
struct layer init_layer(char * name, int previous_size, int current_size);
void free_layer(struct layer l);
struct multi_head_attention_layer *init_multi_head_attention_layer(int masked);
void free_multi_head_attention_layer(struct multi_head_attention_layer *l);
struct feed_forward_layer *init_feed_forward_layer();
void free_feed_forward_layer(struct feed_forward_layer *l);

void soft_max(struct layer *l);
void soft_max_matrix(double **m, int h, int w);
double **attention(int seq_enc, int seq_dec, int mask, double **Q, double **K, double **V, double **Wo);
double **add_and_normalize(double **m1, double **m2, int h, int w, int seq, double *gamma_matrix, double *beta_matrix);
double** attention_add_and_normalize(struct multi_head_attention_layer *l, int seq_enc,
    int seq_dec, double **output_enc, double **output_dec);
double** feed_forward_add_and_normalize(struct feed_forward_layer *l, int seq, double **input);
void sigmoid(struct layer *l);

void forward(struct layer *prev, struct layer *curr);
void backward(struct layer *curr, struct layer *prev);
void update_SGD(struct layer *curr, struct layer *prev);

void print_outputs(struct layer l);
int index_max_output(struct layer l);

double **feed_forward(double **input, int seq, struct layer input_layer, struct layer hidden_layer, struct layer output_layer);
void check(char *path, int expected);

#endif
