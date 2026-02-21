#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/sdl_img.h"
#include "headers/data.h"
#include "headers/nn.h"

//open an image and display it on a window

int main2(void)
{
    srand(time(NULL));

    printf("Hello World!\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (select_random_file() != 0) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    int nb_input_neurons = width*height;

    input_layer = init_layer("input", nb_input_neurons, nb_input_neurons);
    hidden_layer1 = init_layer("hidden", nb_input_neurons, nb_hidden_neurons);
    hidden_layer2 = init_layer("hidden", nb_hidden_neurons, nb_hidden_neurons/2);
    output_layer = init_layer("output", nb_hidden_neurons/2, nb_output_neurons);

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

    int running = 1;

    while (running && c_steps < 100000) {

        //SDL_Surface *screen = SDL_GetWindowSurface(win);

        //display_SDL_matrix(rgba, width, height);
        //SDL_BlitSurface(rgba, NULL, screen, NULL);
        //SDL_UpdateWindowSurface(win);

        /*
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }
        SDL_Delay(5);
        */

        browse();

        free_matrix(raw_data, height);
        raw_data = NULL;
    }

    check("../src/training/3/12.png", 3);

    FILE *f3 = fopen("../src/model.bin", "wb");
    layer_save(f3, &hidden_layer1);
    layer_save(f3, &hidden_layer2);
    layer_save(f3, &output_layer);
    fclose(f3);

    free_layer(input_layer);
    free_layer(hidden_layer1);
    free_layer(hidden_layer2);
    free_layer(output_layer);

    //print_matrix(raw_data, width, height);

    if (rgba) SDL_FreeSurface(rgba);
    if (win) SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
