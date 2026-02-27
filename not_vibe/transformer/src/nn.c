#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/nn.h"
#include "headers/globals.h"
#include "headers/data.h"

struct layer input_layer;
struct layer hidden_layer1;
struct layer hidden_layer2;
struct layer output_layer;

int **convolution(int *input, int **matrix) {
    int **output;
    (void)input;
    (void)matrix;
    return output;
}

void relu(struct layer *l) {
    int cs = l->current_size;
    for (int i = 0; i < cs; i++) {
        if (l->output[i] < 0) {
            l->output[i] = 0;
        }
    }
}

double rand_uniform() {
    return 2.0 * rand() / (double)RAND_MAX - 1.0;
}

struct layer init_layer(char * name, int previous_size, int current_size) {
    struct layer res;
    res.previous_size = previous_size;
    res.current_size = current_size;
    res.name = name;
    if (name[0] != 'i') {
        res.weights = malloc(previous_size * current_size * sizeof(double));
        double scale = sqrt(6.0 / (previous_size + current_size));
        for (int i = 0; i < previous_size * current_size; i++)
        {
            res.weights[i] = rand_uniform() * scale;
        }

        res.biases = malloc(current_size * sizeof(double));
        for (int i = 0; i < current_size; i++) {
            res.biases[i] = 0.01;
        }

        res.output = malloc(current_size * sizeof(double));
        res.delta = malloc(current_size * sizeof(double));
        res.z = malloc(current_size * sizeof(double));
    }
    else {
        res.weights = NULL;
        res.biases = NULL;
        res.output = malloc(current_size * sizeof(double));
        res.delta = NULL;
        res.z = NULL;
    }
    return res;
}

void free_layer(struct layer l) {
        if (l.weights)
            free(l.weights);
        if (l.biases)
            free(l.biases);
        if (l.output)
            free(l.output);
        if (l.delta)
            free(l.delta);
        if (l.z)
            free(l.z);
}

void print_outputs(struct layer l) {
    int cs = l.current_size;
    for (int i = 0; i < cs; i++) {
        printf("%d: %lf\n", i, (l.output)[i]);
    }
}

void put_in_output_matrix(double *input, int W, int H) {

    int c = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            (input_layer.output)[c] = input[y*W + x];
            c++;
        }
    }
}

int index_max_output(struct layer l) {
    int cs = l.current_size;
    int maxi = 0;
    for (int i = 0; i < cs; i++) {
        if (l.output[maxi] < l.output[i])
            maxi = i;
    }
    return maxi;
}

void update_learning_coeff() {
    if (c_steps > 10000) {
        if (c_steps % 3000 == 0) {
            learning_coeff *= 0.98;
            if (learning_coeff < 0.0005) learning_coeff = 0.0005;
        }
    }
}

void soft_max(struct layer *l) {
    int cs = l->current_size;
    double max = l->output[0];
    for (int i = 1; i < cs; i++) {
        if (max < l->output[i]) {
            max = l->output[i];
        }
    }
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum += exp(l->output[i] - max);
    }
    for (int i = 0; i < cs; i++) {
        l->output[i] = exp(l->output[i] - max) / sum;
    }
}

void sigmoid(struct layer *l) {
    int cs = l->current_size;
    for (int i = 0; i < cs; i++) {
        l->output[i] = 1./(1. + exp(-(l->output)[i]));
    }
}

void forward(struct layer *prev, struct layer *curr) {
    int cs = curr->current_size;
    int ps = curr->previous_size;
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum = curr->biases[i];
        for (int j = 0; j < ps; j++) {
            sum += prev->output[j] * curr->weights[i*ps + j];
        }
        curr->z[i] = sum;
        curr->output[i] = sum;
    }
}

double relu_prime(double v) {
    if (v <= 0)
        return 0;
    return 1.0;
}

void backward(struct layer *curr, struct layer *prev) {
    int cs = curr->current_size;
    int ps = curr->previous_size;

    if (!prev->delta) return;

    double s = 0;
    for (int j = 0; j < ps; j++) {
        s = 0.0;
        for (int i = 0; i < cs; i++) {
            s += curr->weights[i*ps + j] * curr->delta[i];
        }
        if (prev->z) {
            prev->delta[j] = s * relu_prime(prev->z[j]);
        } else {
            prev->delta[j] = s;
        }
    }
}

void update_SGD(struct layer *curr, struct layer *prev) {
    int cs = curr->current_size;
    int ps = curr->previous_size;

    double clip = 1.0;     // or 5.0
    double lambda = 1e-4;  // weight decay
    double grad;
    double d;

    for (int i = 0; i < cs; i++) {
        d = curr->delta[i];

        //clip delta (to not have huge deltas)
        if (d > clip) d = clip;
        if (d < -clip) d = -clip;

        for (int j = 0; j < ps; j++) {
            int idx = i*ps + j;

            grad = d * prev->output[j];

            //L2 regularization
            curr->weights[idx] -= learning_coeff * (grad + lambda * curr->weights[idx]);
        }

        curr->biases[i] -= learning_coeff * d;
    }
}

void browse(double *input) {

    put_in_output_matrix(input, 28, 28);

    forward(&input_layer, &hidden_layer1);
    relu(&hidden_layer1);
    forward(&hidden_layer1, &hidden_layer2);
    relu(&hidden_layer2);
    forward(&hidden_layer2, &output_layer);
    soft_max(&output_layer);


    double p = output_layer.output[expected];
    if (p < 1e-15) p = 1e-15;
    loss = -log(p);
    int idx_max_output = index_max_output(output_layer);
    int result = 0;
    if (idx_max_output == expected)
        result = 1;
    printf("loss: %f, value: %f, steps: %d, expected: %c, get: %c, result: %d, lr: %f\n", loss, output_layer.output[expected], c_steps++, expected, idx_max_output, result, learning_coeff);

    //print_outputs(output_layer);

    for (int i = 0; i < output_layer.current_size; i++) {
        output_layer.delta[i] = output_layer.output[i];
    }
    output_layer.delta[expected] -= 1.0; //cross entropy

    backward(&output_layer, &hidden_layer2);
    backward(&hidden_layer2, &hidden_layer1);
    backward(&hidden_layer1, &input_layer);

    update_SGD(&output_layer,  &hidden_layer2);
    update_SGD(&hidden_layer2, &hidden_layer1);
    update_SGD(&hidden_layer1, &input_layer);

    update_learning_coeff();
}
