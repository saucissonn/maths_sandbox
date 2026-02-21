#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "headers/image.h"
#include "headers/matrix.h"
#include "headers/segment.h"
#include "headers/display.h"
#include "headers/fs.h"

static SDL_Window *win = NULL;
static SDL_Surface *rgba = NULL;

static int width = 900;
static int height = 600;
static int top = 300;

int main(void) {
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

    int Hbin = 0, Wbin = 0;
    const int thr = 128;
    double **raw = surface_to_binary_matrix(rgba, top, thr, &Hbin, &Wbin);
    if (!raw) {
        printf("binarize failed\n");
        SDL_DestroyWindow(win);
        IMG_Quit(); SDL_Quit();
        return 1;
    }

    segment_and_print(raw, Hbin, Wbin);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = 0;

                if (e.key.keysym.sym == SDLK_SPACE) {
                    free_matrix(raw, Hbin);
                    if (select_random_file_and_load(&rgba, &width, &height) != 0) { running = 0; break; }
                    SDL_SetWindowSize(win, width, height);

                    raw = surface_to_binary_matrix(rgba, top, thr, &Hbin, &Wbin);
                    if (!raw) { running = 0; break; }
                    segment_and_print(raw, Hbin, Wbin);
                }
            }
        }

        blit_black_white(win, rgba, thr);
        SDL_Delay(16);
    }

    printf("ok\n");

    free_matrix(raw, Hbin);
    if (rgba) SDL_FreeSurface(rgba);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}