// 67 + ratio. -Cam

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/data.h"
#include "headers/nn.h"
#include "headers/fs.h"
#include "headers/matrix.h"
#include "headers/token.h"

//open an image and display it on a window

int Hbin = 0, Wbin = 0;
const int thr = 190;
int out_n = 0;
int running = 1;
int training = 1;
int raw_size = 28;

void train() {
    select_random_file_and_answer();
    printf("input=%s\n", input);
    printf("ans=%s\n", answer);
}

int main(void)
{
    srand((unsigned)time(NULL));

    /*
    FILE *f2 = fopen("../src/model.bin", "rb");

    fseek(f2, 0, SEEK_END);
    long size = ftell(f2);
    rewind(f2);

    if (size == 0) {
        fclose(f2);
    }
    else {
        layer_load(f2, &hidden_layer1);
        layer_load(f2, &hidden_layer2);
        layer_load(f2, &output_layer);
        fclose(f2);
    }
    */


    /*
    while (running) {
        train();
    } */

    init_tokens();

    /*
    char text[255];
    printf("Type sentence up to 255 char: ");
    fgets(text, sizeof(text), stdin);
    */

    select_random_file_and_answer();
    int seq_encoder = 0;
    token **toks_in = str_to_tokens(input, &seq_encoder);

    if (!toks_in) {
        printf("Tokenisation failed\n");
        return 1;
    }

    for (int i = 0; i < seq_encoder; i++) {
        printf("Token %d: ", i+1);
        printf("content = %s", toks_in[i]->content);
        printf("\tid = %d\n", toks_in[i]->id);
    }

    input_matrix_encoder = malloc(seq_encoder * sizeof(double *));
    for (int i = 0; i < seq_encoder; i++) {
        input_matrix_encoder[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            input_matrix_encoder[i][j] = (toks_in[i]->embedding)[j];
        }
    }

    int seq_decoder = 0;
    token **toks_out = str_to_tokens(answer, &seq_decoder);

    if (!toks_out) {
        printf("Tokenisation failed for answer\n");
        return 1;
    }

    for (int i = 0; i < seq_decoder; i++) {
        printf("Token %d: ", i+1);
        printf("content = %s", toks_out[i]->content);
        printf("\tid = %d\n", toks_out[i]->id);
    }

    input_matrix_decoder = malloc(seq_decoder * sizeof(double *));

    for (int i = 0; i < seq_decoder; i++) {
        input_matrix_decoder[i] = malloc(dmodel * sizeof(double));
        for (int j = 0; j < dmodel; j++) {
            input_matrix_decoder[i][j] = (toks_out[i]->embedding)[j];
        }
    }

    struct multi_head_attention_layer *multi_head_attention_layer_encoder = init_multi_head_attention_layer(0);
    struct feed_forward_layer *feed_forward_layer_encoder = init_feed_forward_layer();
    struct multi_head_attention_layer *multi_head_attention_layer_decoder1 = init_multi_head_attention_layer(1);
    struct multi_head_attention_layer *multi_head_attention_layer_decoder2 = init_multi_head_attention_layer(0);
    struct feed_forward_layer *feed_forward_layer_decoder = init_feed_forward_layer();
    
    // start encoder
    double **after_norm_encoder = attention_add_and_normalize(
        multi_head_attention_layer_encoder, seq_encoder, seq_encoder,
        input_matrix_encoder, input_matrix_encoder);
    
    double **output_encoder = feed_forward_add_and_normalize(feed_forward_layer_encoder, 
        seq_encoder, after_norm_encoder);
    // end encoder

    //start decoder
    double **after_norm_decoder1 = attention_add_and_normalize(
        multi_head_attention_layer_decoder1, seq_decoder, seq_decoder,
        input_matrix_decoder, input_matrix_decoder);

    double **after_norm_decoder2 = attention_add_and_normalize(
        multi_head_attention_layer_decoder1, seq_encoder, seq_decoder,
        output_encoder, after_norm_decoder1);

    double **output_decoder = feed_forward_add_and_normalize(feed_forward_layer_decoder, 
        seq_decoder, after_norm_decoder2);
    //end decoder

    double **final = malloc(seq_decoder * sizeof(double *));
    for (int i = 0; i < seq_decoder; i++) {
        final[i] = malloc(vocab_size * sizeof(double));
        for (int j = 0; j < vocab_size; j++) {
            final[i][j] = 0;
        }
    }
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
    for (int i = 0; i < seq_decoder; i++) {
        for (int j = 0; j < vocab_size; j++) {
            for (int k = 0; k < dmodel; k++) {
                final[i][j] += output_decoder[i][k] + W_last[k][j];
            }
            final[i][j] += b_last[j];
        }
    }

    soft_max_matrix(final, seq_decoder, vocab_size);
    
    //print_matrix(final, seq_decoder, vocab_size);

    free_multi_head_attention_layer(multi_head_attention_layer_encoder);
    free_feed_forward_layer(feed_forward_layer_encoder);
    free_multi_head_attention_layer(multi_head_attention_layer_decoder1);
    free_multi_head_attention_layer(multi_head_attention_layer_decoder2);
    free_feed_forward_layer(feed_forward_layer_decoder);
    
    free_matrix(after_norm_encoder, seq_encoder);
    free_matrix(output_encoder, seq_encoder);
    free_matrix(input_matrix_encoder, seq_encoder); 
    free_matrix(input_matrix_decoder, seq_decoder);
    free_matrix(after_norm_decoder1, seq_decoder);
    free_matrix(after_norm_decoder2, seq_decoder);
    free_matrix(final, seq_decoder);
    free_matrix(W_last, dmodel);
    free(b_last);
    free_tokens(toks_in, seq_encoder);
    free_tokens(toks_out, seq_decoder);
    free_vocab();

    /*
    FILE *f3 = fopen("../src/model.bin", "wb");
    layer_save(f3, &hidden_layer1);
    layer_save(f3, &hidden_layer2);
    layer_save(f3, &output_layer);
    fclose(f3);
    */

    //print_matrix(raw_data, width, height);
    printf("\nCLOSE\n");
    return 0;
}
