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

int main(void)
{
    srand(time(NULL)); 

    init_tokens();
    struct multi_head_attention_layer *multi_head_attention_layer_encoder = init_multi_head_attention_layer(0);
    struct feed_forward_layer *feed_forward_layer_encoder = init_feed_forward_layer();
    struct multi_head_attention_layer *multi_head_attention_layer_decoder1 = init_multi_head_attention_layer(1);
    struct multi_head_attention_layer *multi_head_attention_layer_decoder2 = init_multi_head_attention_layer(0);
    struct feed_forward_layer *feed_forward_layer_decoder = init_feed_forward_layer();
    init_linear();


    dWvocab_embed_input = malloc(vocab_size * sizeof(double *));
    dWvocab_embed_answer = malloc(vocab_size * sizeof(double *));

    for (int i = 0; i < vocab_size; i++) {
        dWvocab_embed_input[i] = calloc(dmodel, sizeof(double));
        dWvocab_embed_answer[i] = calloc(dmodel, sizeof(double));
    }

    double **final_output;
    double **grad_first;
    double **grad_linear;

    int seq_encoder = 0;
    int seq_decoder = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    int c = 0;
    while (c < 10000) {
        c++;
        seq_encoder = 0;
        seq_decoder = 0;

        select_random_file_and_answer(&seq_encoder, &seq_decoder);
        printf("encoder: %d, decoder: %d\n", seq_encoder, seq_decoder);
        // start encoder
        attention_add_and_normalize(
            multi_head_attention_layer_encoder, seq_encoder, seq_encoder,
            input_matrix_encoder, input_matrix_encoder
        );

        feed_forward_add_and_normalize(feed_forward_layer_encoder, 
            seq_encoder, multi_head_attention_layer_encoder->norm->Z_normalized);
        // end encoder

        //start decoder
        attention_add_and_normalize(
            multi_head_attention_layer_decoder1, seq_decoder, seq_decoder,
            input_matrix_decoder, input_matrix_decoder);

        attention_add_and_normalize(
            multi_head_attention_layer_decoder2, seq_encoder, seq_decoder,
            feed_forward_layer_encoder->norm->Z_normalized,
            multi_head_attention_layer_decoder1->norm->Z_normalized);

        feed_forward_add_and_normalize(feed_forward_layer_decoder, 
            seq_decoder, multi_head_attention_layer_decoder2->norm->Z_normalized);
        //end decoder
        
        final_output = linear(feed_forward_layer_decoder->norm->Z_normalized,
            seq_decoder);

        grad_first = malloc(seq_decoder * sizeof(double*));
        for (int i = 0; i < seq_decoder; i++) {
            grad_first[i] = malloc(vocab_size * sizeof(double));
        }

        printf("step: %d, loss=%f, time: ", c, cross_entropy(final_output, grad_first, seq_decoder));
        //print_matrix(final_output, seq_decoder, vocab_size);

        grad_linear = backward_linear(feed_forward_layer_decoder->norm->Z_normalized,
            grad_first, seq_decoder
        );

        backward_feed_forward_add_and_normalize(
            feed_forward_layer_decoder, grad_linear, seq_decoder
        );

        backward_attention_add_and_normalize(
            multi_head_attention_layer_decoder2, feed_forward_layer_decoder->dx,
            seq_encoder, seq_decoder
        );

        backward_attention_add_and_normalize(
            multi_head_attention_layer_decoder1,
            multi_head_attention_layer_decoder2->dXdec,
            seq_decoder, seq_decoder
        );

        for (int i = 0; i < vocab_size; i++) {
            for (int j = 0; j < dmodel; j++) {
                dWvocab_embed_answer[i][j] = 0.0;
            }
        }
        
        for (int i = 0; i < seq_decoder; i++) {
            int tok = answer[i];
            for (int j = 0; j < dmodel; j++) {
                dWvocab_embed_answer[tok][j] += multi_head_attention_layer_decoder1->dXdec[i][j];
            }
        }

        for (int i = 0; i < seq_decoder; i++) {
            int tok = answer[i];
            for (int j = 0; j < dmodel; j++) {
                Wvocab_embed_answer[tok][j] -= learning_coeff * dWvocab_embed_answer[tok][j];
            }
        }
        
        backward_feed_forward_add_and_normalize(
            feed_forward_layer_encoder, multi_head_attention_layer_decoder2->dXenc, seq_encoder
        );
        
        backward_attention_add_and_normalize(
            multi_head_attention_layer_encoder,
            feed_forward_layer_encoder->dx,
            seq_encoder, seq_encoder
        );

        //print_matrix(multi_head_attention_layer_encoder->dXenc, seq_encoder, dmodel);

        for (int i = 0; i < vocab_size; i++) {
            for (int j = 0; j < dmodel; j++) {
                dWvocab_embed_input[i][j] = 0.0;
            }
        }
        
        for (int i = 0; i < seq_encoder; i++) {
            int tok = input[i];
            for (int j = 0; j < dmodel; j++) {
                dWvocab_embed_input[tok][j] += multi_head_attention_layer_encoder->dXenc[i][j];
            }
        }

        for (int i = 0; i < seq_encoder; i++) {
            int tok = input[i];
            for (int j = 0; j < dmodel; j++) {
                Wvocab_embed_input[tok][j] -= learning_coeff * dWvocab_embed_input[tok][j];
            }
        }

        printf_time_diff(start, end);
    //print_matrix(final_output, seq_decoder, vocab_size);

    free_matrix(final_output, seq_decoder);
    free_matrix(grad_first, seq_decoder);
    
    }

    free_matrix(input_matrix_encoder, seq_encoder); 
    free_matrix(input_matrix_decoder, seq_decoder);

    free_matrix(W_last, dmodel);
    free(expected_matrix);
    free(b_last);
    free_vocab();
    free_multi_head_attention_layer(multi_head_attention_layer_encoder);
    free_feed_forward_layer(feed_forward_layer_encoder, seq_encoder);
    free_multi_head_attention_layer(multi_head_attention_layer_decoder1);
    free_multi_head_attention_layer(multi_head_attention_layer_decoder2);
    free_feed_forward_layer(feed_forward_layer_decoder, seq_decoder);
    printf("\nCLOSE\n");
    return 0;
}
