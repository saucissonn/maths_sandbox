#include "headers/sdl_img.h"
#include "headers/globals.h"

#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Surface * create_rgba(char * image) {
    //give rgba of an image to extract info of each pixel
    SDL_Surface *img = IMG_Load(image);
    if (!img) {
        printf("IMG_Load error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);

    if (!rgba) {
        printf("SDL_ConvertSurfaceFormat error: %s\n", SDL_GetError());
        SDL_FreeSurface(img);
        return NULL;
    }

    SDL_FreeSurface(img);
    return rgba;
}

int create_window(char * image) {
    //init, create window and rgba
    if (rgba) {
        SDL_FreeSurface(rgba);
        rgba = NULL;
    }

    rgba = create_rgba(image);
    if (!rgba) {
        return 1;
    }

    //width = rgba->w; //fixed size
    //height = rgba->h;

    /*
    win = SDL_CreateWindow(
        "png pixels",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        0
    );
    if (!win) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_FreeSurface(rgba);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    */
    return 0;
}

void display_SDL_matrix(SDL_Surface * input, int width, int height) {
    //ensure rgba is locked, unless we don't know the behavior
    if (SDL_MUSTLOCK(input)) {
        if (SDL_LockSurface(input) != 0) {
            printf("SDL_LockSurface error: %s\n", SDL_GetError());
            //SDL_DestroyWindow(win);
            SDL_FreeSurface(input);
            IMG_Quit();
            SDL_Quit();
            return;
        }
    }
    Uint8 *base = (Uint8*)input->pixels;
    int pitch = input->pitch;
    int average;
    Uint8 *row;
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    for (int y = 0; y < height; y++) {
        row = base + y * pitch; //row modifies input, because it points to it
        for (int x = 0; x < width; x++) {
            r = row[x*4 + 0];
            g = row[x*4 + 1];
            b = row[x*4 + 2];
            a = row[x*4 + 3];

            average = (r+g+b)/3; //to invert black and white do 255 - the value

            row[x*4 + 0] = average;
            row[x*4 + 1] = average;
            row[x*4 + 2] = average;
            row[x*4 + 3] = a;
        }
    }
    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input); //like a "free()" because we don't need it anymore
}

double **convert_surface_to_double(SDL_Surface * input) {
    //ensure rgba is locked, unless we don't know the behavior
    int pitch = input->pitch;
    int w = input->w;
    double **output = malloc(pitch*sizeof(*output));
    for(int i = 0; i < pitch; i++)
        output[i] = malloc(w * sizeof(*(output[i])));
    Uint8 *base = (Uint8*)input->pixels;
    Uint8 *row;
    double v;
    Uint8 r;
    Uint8 g;
    Uint8 b;

    for (int y = 0; y < height; y++) {
        row = base + y * pitch; //row modifies input, because it points to it
        for (int x = 0; x < width; x++) {
            r = row[x*4 + 0];
            g = row[x*4 + 1];
            b = row[x*4 + 2];

            v = (r+g+b)/765.0;

            output[y][x] = v;
        }
    }
    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input); //like a "free()" because we don't need it anymore
    return output;
}
