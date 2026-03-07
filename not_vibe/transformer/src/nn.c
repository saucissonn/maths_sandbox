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

struct add_and_normalize_layer *init_add_and_normalize_layer() {
    struct add_and_normalize_layer *res = malloc(sizeof(struct add_and_normalize_layer));
    res->Z = NULL;
    res->Z_normalized = NULL;

    res->gamma = malloc(dmodel * sizeof(double));
    res->beta = malloc(dmodel * sizeof(double));

    for (int i = 0; i < dmodel; i++) {
        (res->gamma)[i] = 1.0;
        (res->beta)[i]  = 0.0;
    }
    return res;
}

void free_add_and_normalize_layer(struct add_and_normalize_layer *l) {
    if (!l) return;
    free(l->gamma);
    free(l->beta);
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
        res.delta = calloc(current_size, sizeof(double));
        res.z = malloc(current_size * sizeof(double));
    }
    else {
        res.weights = NULL;
        res.biases = NULL;
        res.output = malloc(current_size * sizeof(double));
        res.delta = calloc(current_size, sizeof(double));
        res.z = NULL;
    }
    res.sum_delta = calloc(current_size, sizeof(double));
    return res;
}

void free_layer(struct layer l) {
    free(l.weights);
    free(l.biases);
    free(l.output);
    free(l.delta);
    free(l.sum_delta);
    free(l.z);
}

struct multi_head_attention_layer *init_multi_head_attention_layer(int masked) { //create Wq, Wk, Wv, Wo, Q, K, V, gamma and beta
    struct multi_head_attention_layer *res = malloc(sizeof(struct multi_head_attention_layer));
    res->nb_heads = 4;
    int dk = dmodel/res->nb_heads; //4 heads
    double limit = sqrt(6.0 / (dmodel + dk));
    
    res->masked = masked;
    res->Xenc = NULL;
    res->Xdec = NULL;
    res->dXenc = NULL;
    res->dXdec = NULL;
    res->Wq = malloc(dmodel * sizeof(double *));
    res->Wk = malloc(dmodel * sizeof(double *));
    res->Wv = malloc(dmodel * sizeof(double *));
    res->Wo = malloc(dmodel * sizeof(double *));

    res->Q = NULL;
    res->K = NULL; 
    res->V = NULL;
    res->A = malloc(dmodel * sizeof(double **));
    res->S = malloc(dmodel * sizeof(double **));
    res->concat_heads = malloc(dmodel * sizeof(double *));
    for (int i = 0; i < res->nb_heads; i++) { //4 heads
        (res->A)[i] = NULL;
        (res->S)[i] = NULL;
    }

    res->norm = init_add_and_normalize_layer();

    for (int i = 0; i < dmodel; i++) {
        (res->Wq)[i] = malloc(dmodel * sizeof(double));
        (res->Wk)[i] = malloc(dmodel * sizeof(double));
        (res->Wv)[i] = malloc(dmodel * sizeof(double));
        (res->Wo)[i] = malloc(dmodel * sizeof(double));
        (res->concat_heads)[i] = NULL;
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
    free_add_and_normalize_layer(l->norm);
}


struct feed_forward_layer *init_feed_forward_layer() {
    struct feed_forward_layer *res = malloc(sizeof(struct feed_forward_layer));
    
    res->input_layer = init_layer("input", dmodel, dmodel*4);
    res->hidden_layer = init_layer("hidden", dmodel*4, dmodel);
    res->output_layer = init_layer("output", dmodel, dmodel);

    res->norm = init_add_and_normalize_layer();

    res->dx = NULL;

    return res;
}

void free_feed_forward_layer(struct feed_forward_layer *l, int seq) {
    if (!l) return;
    free_layer(l->input_layer);
    free_layer(l->hidden_layer);
    free_layer(l->output_layer);
    free_add_and_normalize_layer(l->norm);
}

void init_linear() {
    int dk = dmodel/4; //4 heads
    double limit = sqrt(6.0 / (dmodel + dk));
    W_last = malloc(dmodel * sizeof(double *));
    for (int i = 0; i < dmodel; i++) {
        W_last[i] = malloc(vocab_size * sizeof(double));
        for (int j = 0; j < vocab_size; j++) {
            W_last[i][j] = rand_uniform_token(-limit, limit);
        }
    }
    b_last = malloc(vocab_size * sizeof(double));
    for (int i = 0; i < vocab_size; i++) {
        b_last[i] = rand_uniform_token(-0.001, 0.001);
    }
}

double **linear(double **input, int seq)
{
    double **res = malloc(seq * sizeof(double *));
    for (int i = 0; i < seq; i++)
        res[i] = calloc(vocab_size, sizeof(double));

    for (int i = 0; i < seq; i++)
    {
        for (int j = 0; j < vocab_size; j++)
            res[i][j] = b_last[j];

        for (int k = 0; k < dmodel; k++)
        {
            double v = input[i][k];
            for (int j = 0; j < vocab_size; j++)
                res[i][j] += v * W_last[k][j]; // W^T
        }
    }
    return res;
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

double **soft_max_matrix(double **m, int h, int w) {
    double **res = malloc(h * sizeof(double *));
    for (int i = 0; i < h; i++) {
        res[i] = malloc(w * sizeof(double));

        double max = m[i][0];
        for (int j = 1; j < w; j++) {
            if (m[i][j] > max) max = m[i][j];
        }

        double sum = 0.0;
        for (int j = 0; j < w; j++) {
            sum += exp(m[i][j] - max);
        }

        for (int j = 0; j < w; j++) {
            res[i][j] = exp(m[i][j] - max) / sum;
        }
    }
    return res;
}

double **attention(int seq_enc, int seq_dec, struct multi_head_attention_layer *l) {
    int dk = dmodel/l->nb_heads;
    double **temp_head;

    l->concat_heads = malloc(seq_dec * sizeof(double *));
    l->Xdec = malloc(seq_dec * sizeof(double *));
    for (int i = 0; i < seq_dec; i++) {
        l->concat_heads[i] = malloc(dmodel * sizeof(double));
        l->Xdec[i] = malloc(dmodel * sizeof(double));
    }

    l->Xenc = malloc(seq_enc * sizeof(double *));
    for (int i = 0; i < seq_enc; i++) {
        l->Xenc[i] = malloc(dmodel * sizeof(double));
    }

    double **res;
    for (int i = 0; i < l->nb_heads; i++) {
        (l->S)[i] = matrix_mul_sub(l->Q, 0, seq_dec, dk*i, dk*(i+1),
            l->K, dk*i, dk*(i+1), 0, seq_enc,
            l->masked
        );
        (l->A)[i] = soft_max_matrix((l->S)[i], seq_dec, seq_enc);
        temp_head = matrix_mul((l->A)[i], l->V, seq_dec, seq_enc, dk);
        for (int i2 = 0; i2 < seq_dec; i2++) {
            for (int j = 0; j < dk; j++) {
                l->concat_heads[i2][dk*i + j] = temp_head[i2][j];
            }
        }
        free_matrix(temp_head, seq_dec);
    }

    res = matrix_mul(l->concat_heads, l->Wo, seq_dec, dmodel, dmodel);
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

void attention_add_and_normalize(struct multi_head_attention_layer *l, int seq_enc,
    int seq_dec, double **output_enc, double **output_dec) {

    l->Q = matrix_mul(output_dec, l->Wq, seq_dec, dmodel, dmodel);
    l->K = matrix_mul_transpose(output_enc, l->Wk, seq_enc, dmodel, dmodel); 
    l->V = matrix_mul(output_enc, l->Wv, seq_enc, dmodel, dmodel);

    l->norm->Z = attention(seq_enc, seq_dec, l);
    
    for (int i = 0; i < seq_enc; i++) {
        for (int j = 0; j < dmodel; j++) {
            l->Xenc[i][j] = output_enc[i][j];
        }
    }

    for (int i = 0; i < seq_dec; i++) {
        for (int j = 0; j < dmodel; j++) {
            l->Xdec[i][j] = output_dec[i][j];
        }
    }

    l->norm->Z_normalized = add_and_normalize(l->norm->Z, output_dec, seq_dec, dmodel, seq_dec, 
    l->norm->gamma, l->norm->beta);
}

double** feed_forward_add_and_normalize(struct feed_forward_layer *l, int seq, double **input) {
    
    feed_forward(input, seq, l);

    double** res = add_and_normalize(input, l->norm->Z, seq, dmodel, seq, l->norm->gamma, l->norm->beta);
    l->norm->Z_normalized = malloc(seq * sizeof(double *));
    for (int i = 0; i < seq; i++) {
        l->norm->Z_normalized[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            l->norm->Z_normalized[i][j] = res[i][j];
        }
    }
                
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
        prev->sum_delta[j] += prev->delta[j];
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
        d = curr->sum_delta[i];

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

void feed_forward(double **input, int seq, struct feed_forward_layer *l) {
    double **out = malloc(seq * sizeof(double *));
    l->norm->Z = malloc(seq * sizeof(double *));
    for (int i = 0; i < seq; i++) {
        out[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            out[i][j] = input[i][j];
        }
        put_in_output_matrix(out[i], dmodel, l->input_layer);

        forward(&l->input_layer, &l->hidden_layer);
        relu(&l->hidden_layer);
        forward(&l->hidden_layer, &l->output_layer);
        l->norm->Z[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            l->norm->Z[i][j] = ((l->output_layer).z)[j];
        }
    }
}

double logsumexp(double *z, int n) {
    double m = z[0];
    for (int i = 1; i < n; i++) 
        if (z[i] > m) 
            m = z[i];

    double s = 0.0;
    for (int i = 0; i < n; i++) 
        s += exp(z[i] - m);

    return m + log(s);
}

double cross_entropy(double **input, double **grad, int seq) {
    double loss = 0.0;

    for (int t = 0; t < seq; t++) {
        const int y = expected_matrix[t];

        double lse = logsumexp(input[t], vocab_size);
        loss += -(input[t][y] - lse);

        for (int j = 0; j < vocab_size; j++) {
            double p = exp(input[t][j] - lse);
            grad[t][j] = p;
        }
        grad[t][y] -= 1;
        
    }

    loss /= (double)seq;
    for (int t = 0; t < seq; t++) {
        for (int j = 0; j < vocab_size; j++) {
            grad[t][j] /= (double)seq;
        }
    }

    return loss;
}

void sgd_update_rect(double **W, double *b, double **dW, double *db,
                     int rows, int cols)
{
    double clip = 1.0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double g = dW[i][j];

            if (g > clip) g = clip;
            if (g < -clip) g = -clip;

            W[i][j] -= learning_coeff * g;
        }
    }

    for (int j = 0; j < cols; j++) {
        double g = db[j];

        if (g > clip) g = clip;
        if (g < -clip) g = -clip;

        b[j] -= learning_coeff * g;
    }
}

void sgd_update_row(double *W, double *b, double *dW, double *db) {
    for (int i = 0; i < dmodel; i++) {
        W[i] -= learning_coeff * dW[i];
        b[i] -= learning_coeff * db[i];
    }
}

double **backward_linear(double **input, double **grad, int seq) {
    double **dW = matrix_mul_AT_B(input, grad, seq, dmodel, vocab_size);
    double *db = calloc(vocab_size, sizeof(double));
    for (int j = 0; j < vocab_size; j++) {
        double sum = db[j];
        for (int i = 0; i < seq; i++) {
            sum += grad[i][j];
        }
        db[j] = sum;
    }

    double **res = matrix_mul_A_BT(grad, W_last, seq, vocab_size, dmodel);
    sgd_update_rect(W_last, b_last, dW, db, dmodel, vocab_size);
    free_matrix(dW, dmodel);
    free(db);
    
    return res;
}

double **backward_add_and_normalize(struct add_and_normalize_layer *layer, double **grad, int seq) {
    double *dgamma = calloc(dmodel, sizeof(double));
    double *dbeta = calloc(dmodel, sizeof(double));

    for (int i = 0; i < seq; i++) {
        for (int j = 0; j < dmodel; j++) {
            dgamma[j] += grad[i][j] * layer->Z[i][j];
            dbeta[j] += grad[i][j];
        }
    }

    double **dZ_normalized = malloc(seq * sizeof(double*));
    for (int i = 0; i < seq; i++) {
        dZ_normalized[i] = calloc(dmodel, sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            dZ_normalized[i][j] += grad[i][j] * layer->gamma[j];
        }
    }

    double **dZ = malloc(seq * sizeof(double *));
    double *m1 = calloc(seq, sizeof(double));
    double *m2 = calloc(seq, sizeof(double));
    for (int i = 0; i < seq; i++) {
        m1[i] = mean_row(dZ_normalized[i], dmodel);
        double *temp = calloc(dmodel, sizeof(double));

        for (int j = 0; j < dmodel; j++) {
            temp[j] += dZ_normalized[i][j] * layer->Z_normalized[i][j];
        }

        m2[i] = mean_row(temp, dmodel);
        free(temp);
        dZ[i] = malloc(dmodel * sizeof(double));
        double mu = mean_row(layer->Z[i], dmodel);
        double invstd = 1.0 / sqrt(variance_row(layer->Z[i], dmodel, mu) + 1e-6);

        for (int j = 0; j < dmodel; j++) {
            dZ[i][j] = invstd * (dZ_normalized[i][j] - m1[i] - layer->Z_normalized[i][j] * m2[i]);
        }
    }

    sgd_update_row(layer->gamma, layer->beta, dgamma, dbeta);
    free(dgamma);
    free(dbeta);
    free(m1);
    free(m2);
    free_matrix(dZ_normalized, seq);
    return dZ;
}

void backward_feed_forward_add_and_normalize(struct feed_forward_layer *layer, double **grad, int seq) {
    for (int i = 0; i < layer->input_layer.current_size; i++) {
        layer->input_layer.sum_delta[i] = 0;
    }
    for (int i = 0; i < layer->hidden_layer.current_size; i++) {
        layer->hidden_layer.sum_delta[i] = 0;
    }
    for (int i = 0; i < layer->output_layer.current_size; i++) {
        layer->output_layer.sum_delta[i] = 0;
    }
    double **dZ = backward_add_and_normalize(layer->norm, grad, seq);
    layer->dx = malloc(seq * sizeof(double *)); //grad before entering FFD
    for (int i = 0; i < seq; i++) {
        layer->dx[i] = calloc(dmodel, sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            layer->output_layer.delta[j] = dZ[i][j];
        }
        backward(&layer->output_layer, &layer->hidden_layer);
        backward(&layer->hidden_layer, &layer->input_layer);
        for (int j = 0; j < dmodel; j++) {
            layer->dx[i][j] = layer->input_layer.delta[j] + dZ[i][j];
        }
    }
    update_SGD(&layer->output_layer, &layer->hidden_layer);
    update_SGD(&layer->hidden_layer, &layer->input_layer);
    free_matrix(dZ, seq);
}

double **softmax_backward(double **a, double **da, int seq_enc, int seq_dec, int heads) {
    int dk = dmodel/heads;
    double invsdk = 1. / sqrt(dk);
    double **ds = malloc(seq_dec * sizeof(double *));
    for (int i = 0; i < seq_dec; i++) {
        double dot = 0.0;
        ds[i] = calloc(seq_enc, sizeof(double));
        
        for (int j = 0; j < seq_enc; j++) {
            dot += da[i][j] * a[i][j];
        }

        for (int j = 0; j < seq_enc; j++) {
            ds[i][j] = a[i][j] * (da[i][j] - dot) * invsdk;
        }
    }
    return ds;
}

void sgd_update_sub(double **m, double **grad, int i_start, int i_end,
    int j_start, int j_end
) {
    double clip = 1.0;

    for (int i = i_start; i < i_end; i++) {
        for (int j = j_start; j < j_end; j++) {
            double g = grad[i][j];

            if (g > clip) g = clip;
            if (g < -clip) g = -clip;

            m[i][j] -= learning_coeff * g;
        }
    }
}

void backward_attention_add_and_normalize(
    struct multi_head_attention_layer *layer, double **grad,
    int seq_enc, int seq_dec) {

    int dk = dmodel/layer->nb_heads;
    double **dZ = backward_add_and_normalize(layer->norm, grad, seq_dec);
    double **dH = matrix_mul_A_BT(dZ, layer->Wo, seq_dec, dmodel, dmodel);
    double **dWo = matrix_mul_AT_B(layer->concat_heads, dZ, seq_dec, dmodel, seq_dec);
    layer->dXenc = malloc(seq_enc * sizeof(double *));
    layer->dXdec = malloc(seq_dec * sizeof(double *));
    for (int i = 0; i < seq_enc; i++) {
        layer->dXenc[i] = calloc(dmodel, sizeof(double));
    }
    for (int i = 0; i < seq_dec; i++) {
        layer->dXdec[i] = calloc(dmodel, sizeof(double));
    }

    for (int i = 0; i < layer->nb_heads; i++) {

        double **dVi = matrix_mul_sub_AT(layer->A[i], 0, seq_dec, 0, seq_enc,
                                        dH, 0, seq_dec, dk*i, dk*(i+1), 0
        );

        double **dAi = matrix_mul_sub_BT(dH, 0, seq_dec, dk*i, dk*(i+1),
                                        layer->V, 0, seq_enc, dk*i, dk*(i+1), 0
        );

        double **dSi = softmax_backward(layer->A[i], dAi, seq_enc, seq_dec, layer->nb_heads);

        double **dQi = matrix_mul_sub(dSi, 0, seq_dec, 0, seq_enc,
                                    layer->K, 0, seq_enc, dk*i, dk*(i+1), 0
        );

        double **dKi = matrix_mul_sub_AT(dSi, 0, seq_dec, 0, seq_enc,
                                        layer->Q, 0, seq_dec, dk*i, dk*(i+1), 0
        );

        double **dWqi = matrix_mul_sub_AT(layer->Xdec, 0, seq_dec, 0, dmodel,
                                        dQi, 0, seq_dec, dk*i, dk*(i+1), 0
        );

        double **dWki = matrix_mul_sub_AT(layer->Xenc, 0, seq_enc, 0, dmodel,
                                        dKi, 0, seq_enc, dk*i, dk*(i+1), 0
        );

        double **dWvi = matrix_mul_sub_AT(layer->Xenc, 0, seq_enc, 0, dmodel,
                                        dVi, 0, seq_enc, dk*i, dk*(i+1), 0
        );

        double **temp_dec = matrix_mul_sub_BT(dQi, 0, seq_dec, 0, dk,
                                        layer->Wq, 0, dmodel, dk*i, dk*(i+1), 0
        );

        double **temp_enc1 = matrix_mul_sub_BT(dKi, 0, seq_enc, 0, dk,
                                        layer->Wk, 0, dmodel, dk*i, dk*(i+1), 0
        );

        double **temp_enc2 = matrix_mul_sub_BT(dVi, 0, seq_enc, 0, dk,
                                        layer->Wv, 0, dmodel, dk*i, dk*(i+1), 0
        );

        for (int i = 0; i < seq_dec; i++) {
            for (int j = 0; j < dmodel; j++) {
                layer->dXdec[i][j] += temp_dec[i][j];
            }
        }
        
        for (int i = 0; i < seq_enc; i++) {
            for (int j = 0; j < dmodel; j++) {
                layer->dXenc[i][j] += temp_enc1[i][j] + temp_enc2[i][j];
            }
        }

        sgd_update_sub(layer->Wq, dWqi, 0, dmodel, dk*i, dk*(i+1));
        sgd_update_sub(layer->Wk, dWki, 0, dmodel, dk*i, dk*(i+1));
        sgd_update_sub(layer->Wv, dWvi, 0, dmodel, dk*i, dk*(i+1));
    }

    sgd_update_sub(layer->Wo, dWo, 0, dmodel, 0, dmodel);

    free_matrix(layer->concat_heads, seq_dec);

}
