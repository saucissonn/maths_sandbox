#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "globals.h"
#include "sdl_img.h"
#include "data.h"
#include "nn.h"

//open an image and display it on a window

int main(void)
{
    srand(time(NULL));

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

    int running = 1;

    while (running && c_steps < 50) {

        select_random_file();

        //SDL_Surface *screen = SDL_GetWindowSurface(win);

        //display_SDL_matrix(rgba, width, height);
        //SDL_BlitSurface(rgba, NULL, screen, NULL);
        //SDL_UpdateWindowSurface(win);

        raw_data = convert_surface_to_double(rgba);

        int c = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                (input_layer.output)[c] = raw_data[y][x];
                c++;
            }
        }

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
