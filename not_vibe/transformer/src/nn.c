#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/nn.h"
#include "headers/globals.h"
#include "headers/data.h"
#include "headers/token.h"

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

struct multi_head_attention_layer *init_multi_head_attention_layer(int masked) { //create Wq, Wk, Wv, Wo, Q, K, V, gamma and beta
    struct multi_head_attention_layer *res = malloc(sizeof(struct multi_head_attention_layer));
    int dk = dmodel/4; //4 heads
    double limit = sqrt(6.0 / (dmodel + dk));
    
    res->masked = masked;
    res->Wq = malloc(dmodel * sizeof(double *));
    res->Wk = malloc(dmodel * sizeof(double *));
    res->Wv = malloc(dmodel * sizeof(double *));
    res->Wo = malloc(dmodel * sizeof(double *));

    res->Q = NULL;
    res->K = NULL; 
    res->V = NULL;

    res->gamma = malloc(dmodel * sizeof(double));
    res->beta = malloc(dmodel * sizeof(double));

    for (int i = 0; i < dmodel; i++) {
        (res->Wq)[i] = malloc(dmodel * sizeof(double));
        (res->Wk)[i] = malloc(dmodel * sizeof(double));
        (res->Wv)[i] = malloc(dmodel * sizeof(double));
        (res->Wo)[i] = malloc(dmodel * sizeof(double));
        (res->gamma)[i] = 1.0;
        (res->beta)[i]  = 0.0;
        for (int j = 0; j < dmodel; j++) {
            ((res->Wq)[i])[j] = rand_uniform_token(-limit, limit);
            ((res->Wk)[i])[j] = rand_uniform_token(-limit, limit);
            ((res->Wv)[i])[j] = rand_uniform_token(-limit, limit);
            ((res->Wo)[i])[j] = rand_uniform_token(-limit, limit);
        }
    }
    return res;
}

void free_multi_head_attention_layer(struct multi_head_attention_layer *l) {
    if (!l) return;
    if (l->Wq) {
        for (int i = 0; i < dmodel; i++) {
            free((l->Wq)[i]);
        }
    }
    if (l->Wk) {
        for (int i = 0; i < dmodel; i++) {
            free((l->Wk)[i]);
        }
    }
    if (l->Wv) {
        for (int i = 0; i < dmodel; i++) {
            free((l->Wv)[i]);
        }
    }
    if (l->Wo) {
        for (int i = 0; i < dmodel; i++) {
            free((l->Wo)[i]);
        }
    }
    free(l->gamma);
    free(l->beta);
}


struct feed_forward_layer *init_feed_forward_layer() {
    struct feed_forward_layer *res = malloc(sizeof(struct feed_forward_layer));
    
    res->input_layer = init_layer("input", dmodel, dmodel*4);
    res->hidden_layer = init_layer("hidden", dmodel*4, dmodel);
    res->output_layer = init_layer("output", dmodel, dmodel);

    res->gamma = malloc(dmodel * sizeof(double));
    res->beta = malloc(dmodel * sizeof(double));

    for (int i = 0; i < dmodel; i++) {
        (res->gamma)[i] = 1.0;
        (res->beta)[i]  = 0.0;
    }
    return res;
}

void free_feed_forward_layer(struct feed_forward_layer *l) {
    if (!l) return;
    free_layer(l->input_layer);
    free_layer(l->hidden_layer);
    free_layer(l->output_layer);
    free(l->gamma);
    free(l->beta);
}

void print_outputs(struct layer l) {
    int cs = l.current_size;
    for (int i = 0; i < cs; i++) {
        printf("%d: %lf\n", i, (l.output)[i]);
    }
}

void put_in_output_matrix(double *input, int W, struct layer input_layer) {
    int c = 0;
    for (int x = 0; x < W; x++) {
        (input_layer.output)[c] = input[x];
        c++;
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

void soft_max_matrix(double **m, int h, int w) {
    double max = m[0][0];
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (max < m[i][j]) {
                max = m[i][j];
            }
        }
    }
    double sum = 0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            sum += exp(m[i][j] - max);
        }
    }
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            m[i][j] = exp(m[i][j] - max) / sum;
        }
    }
}

double **attention(int seq_enc, int seq_dec, int mask, double **Q, double **K, double **V, double **Wo) {
    int dk = dmodel/4;
    double **temp_sm;
    double **temp_head;
    double **temp_res = malloc(seq_dec * sizeof(double *));
    for (int i = 0; i < seq_dec; i++) {
        temp_res[i] = malloc(dmodel * sizeof(double));
    }
    double **res;
    for (int i = 0; i < 4; i++) {
        temp_sm = matrix_mul_sub(Q, 0, seq_dec, dk*i, dk*(i+1),
            K, dk*i, dk*(i+1), 0, seq_enc,
            mask
        );
        soft_max_matrix(temp_sm, seq_dec, seq_enc);
        temp_head = matrix_mul(temp_sm, V, seq_dec, seq_enc, dk);
        free_matrix(temp_sm, seq_dec);
        for (int i2 = 0; i2 < seq_dec; i2++) {
            for (int j = 0; j < dk; j++) {
                temp_res[i2][dk*i + j] = temp_head[i2][j];
            }
        }
        free_matrix(temp_head, seq_dec);
    }
    res = matrix_mul(temp_res, Wo, seq_dec, dmodel, dmodel);
    free_matrix(temp_res, seq_dec);
    return res;
}

double **add_and_normalize(double **m1, double **m2, int h, int w, int seq, double *gamma_matrix, double *beta_matrix) {
    double **y = matrix_sum(m1, m2, seq, dmodel);
    double **out = malloc(seq * sizeof(double *));
    for (int i = 0; i < seq; i++) {
        out[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            out[i][j] = 0;
        }
    }

    for (int i = 0; i < seq; i++) {
        double mean = mean_row(y[i], dmodel);
        double var = variance_row(y[i], dmodel, mean);

        for (int j = 0; j < dmodel; j++) {
            double norm = (y[i][j] - mean) / sqrt(var + 1e-5);
            out[i][j] = gamma_matrix[j] * norm + beta_matrix[j];
        }
    }
    free_matrix(y, seq);
    return out;
}

double** attention_add_and_normalize(struct multi_head_attention_layer *l, int seq_enc,
    int seq_dec, double **output_enc, double **output_dec) {
    
    l->Q = matrix_mul(output_dec, l->Wq, seq_dec, dmodel, dmodel);
    l->K = matrix_mul_transpose(output_enc, l->Wk, seq_enc, dmodel, dmodel); 
    l->V = matrix_mul(output_enc, l->Wv, seq_enc, dmodel, dmodel);
    
    double **MH_A = attention(seq_enc, seq_dec, l->masked, l->Q, l->K, l->V, l->Wo);

    double** res = add_and_normalize(MH_A, output_dec, seq_dec, dmodel, seq_dec, 
    l->gamma, l->beta);
                
    return res;
}

double** feed_forward_add_and_normalize(struct feed_forward_layer *l, int seq, double **input) {
    
    double **after_ff = feed_forward(input, seq, l->input_layer, l->hidden_layer, l->output_layer);

    double** res = add_and_normalize(input, after_ff, seq, dmodel, seq, l->gamma, l->beta);
                
    return res;
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

double **feed_forward(double **input, int seq, struct layer input_layer, struct layer hidden_layer, struct layer output_layer) {
    double **out = malloc(seq * sizeof(double *));
    for (int i = 0; i < seq; i++) {
        out[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < seq; j++) {
            out[i][j] = input[i][j];
        }
        put_in_output_matrix(out[i], dmodel, input_layer);

        forward(&input_layer, &hidden_layer);
        relu(&hidden_layer);
        forward(&hidden_layer, &output_layer);
    }
    /*
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

    backward(&output_layer, &hidden_layer1);
    backward(&hidden_layer1, &input_layer);

    update_SGD(&output_layer, &hidden_layer1);
    update_SGD(&hidden_layer1, &input_layer);

    update_learning_coeff();
    */
   return out;
}
