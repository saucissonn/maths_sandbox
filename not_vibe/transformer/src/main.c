// 67 + ratio. -Cam

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/data.h"
#include "headers/nn.h"
#include "headers/fs.h"
#include "headers/matrix.h"

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

    int nb_input_neurons = raw_size*raw_size;

    input_layer = init_layer("input", nb_input_neurons, nb_input_neurons);
    hidden_layer1 = init_layer("hidden", nb_input_neurons, nb_hidden_neurons);
    hidden_layer2 = init_layer("hidden", nb_hidden_neurons, nb_hidden_neurons/2);
    output_layer = init_layer("output", nb_hidden_neurons/2, nb_output_neurons);

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



    while (running) {
        train();
    }
    printf("\nCLOSE\n");

    /*
    FILE *f3 = fopen("../src/model.bin", "wb");
    layer_save(f3, &hidden_layer1);
    layer_save(f3, &hidden_layer2);
    layer_save(f3, &output_layer);
    fclose(f3);
    */

    free_layer(input_layer);
    free_layer(hidden_layer1);
    free_layer(hidden_layer2);
    free_layer(output_layer);

    //print_matrix(raw_data, width, height);
    return 0;
}
