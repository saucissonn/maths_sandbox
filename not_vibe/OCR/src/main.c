// 67 + ratio. -Cam

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/sdl_img.h"
#include "headers/data.h"
#include "headers/nn.h"
#include "headers/fs.h"
#include "headers/image.h"
#include "headers/matrix.h"
#include "headers/segment.h"
#include "headers/display.h"

//open an image and display it on a window

int Hbin = 0, Wbin = 0;
const int thr = 190;
int out_n = 0;
int running = 1;
int training = 1;
int raw_size = 28;

void train() {
    free_matrix(raw_data, raw_size);
    if (select_random_file_and_load(&rgba, &width, &height) != 0) running = 0;
    Hbin = 0, Wbin = 0;
    out_n = 0;
    raw_data = surface_to_binary_matrix(rgba, top, thr, &Hbin, &Wbin);
    raw_data = segment_to_matrix_28x28(raw_data, Wbin, Hbin, &out_n);
    blit_black_white(win, rgba, thr); //display on screen
    rewind(file_ans);
    for (int i = 0; i < out_n; i++) {
        expected = char_to_index(fgetc(file_ans));
        browse(raw_data[i]);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    win = SDL_CreateWindow("Segmentation (lines + chars) -> 28x28",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           width, height, 0);
    if (!win) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        IMG_Quit(); SDL_Quit();
        return 1;
    }

    if (select_random_file_and_load(&rgba, &width, &height) != 0) {
        SDL_DestroyWindow(win);
        IMG_Quit(); SDL_Quit();
        return 1;
    }

    SDL_SetWindowSize(win, width, height);
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

    raw_data = surface_to_binary_matrix(rgba, top, thr, &Hbin, &Wbin);
    if (!raw_data) {
        printf("binarize failed\n");
        SDL_DestroyWindow(win);
        IMG_Quit(); SDL_Quit();
        return 1;
    }

    //segment_and_print(raw_data, Hbin, Wbin);

    int out_n = 0;
    raw_data = segment_to_matrix_28x28(raw_data, Wbin, Hbin, &out_n);
    printf("out n: %d\n", out_n);

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = 0;

                if (e.key.keysym.sym == SDLK_SPACE) {
                    //SDL_SetWindowSize(win, width, height);
                    training *= -1;
                    if (!raw_data) { running = 0; break; }
                }
            }
        }
        if (training == 1)
            train();
        SDL_Delay(16); //60 FPS
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

    free_matrix(raw_data, Hbin);
    if (rgba) SDL_FreeSurface(rgba);
    if (win) SDL_DestroyWindow(win);
    if (file_ans) fclose(file_ans);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
