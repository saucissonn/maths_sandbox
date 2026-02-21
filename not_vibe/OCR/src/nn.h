#ifndef NN_H
#define NN_H

struct layer {
    char *name;
    int previous_size;
    int current_size;  //16
    double *weights;   //matrix (output_size × input_size)
    double *biases;    //output_size
    double *output;    //activations
    double *delta;     //gradients
    double *z;
};

extern struct layer input_layer;
extern struct layer hidden_layer1;
extern struct layer hidden_layer2;
extern struct layer output_layer;

void relu(struct layer *l);
struct layer init_layer(char * name, int previous_size, int current_size);
void free_layer(struct layer l);

void soft_max(struct layer *l);
void sigmoid(struct layer *l);

void forward(struct layer *prev, struct layer *curr);
void backward(struct layer *curr, struct layer *prev);
void update_SGD(struct layer *curr, struct layer *prev);

void print_outputs(struct layer l);
int index_max_output(struct layer l);

void f_and_b(void);

#endif
