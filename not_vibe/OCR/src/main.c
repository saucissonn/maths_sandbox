#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>

//open an image and display it on a window

SDL_Surface *img;
SDL_Surface *rgba;

int create_window() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    return 0;
}

SDL_Surface * create_rgba(char * image) {
    SDL_Surface *img = IMG_Load(image);
    if (!img) {
        printf("IMG_Load error: %s\n", IMG_GetError());
        IMG_Quit();
        SDL_Quit();
        return NULL;
    }
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);

    if (!rgba) {
        printf("SDL_ConvertSurfaceFormat error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return NULL;
    }

    SDL_FreeSurface(img);
    return rgba;
}

int **convert_surface_to_int(SDL_Surface * input) {
    int **output = malloc((input->h)*sizeof(*output));
    for(int i = 0; i < (input->h); i++)
        output[i] = malloc((input->w) * sizeof(*(output[i])));
    Uint8 *base = (Uint8*)input->pixels;
    int pitch = input->pitch;
    int average;

    for (int y = 0; y < input->h; y++) {
        Uint8 *row = base + y * pitch; //row modifies input, because it points to it
        for (int x = 0; x < input->w; x++) {
            Uint8 r = row[x*4 + 0];
            Uint8 g = row[x*4 + 1];
            Uint8 b = row[x*4 + 2];
            Uint8 a = row[x*4 + 3];

            average = (r+g+b)/3;

            output[y][x] = average;
        }
    }
    return output;
}

void free_matrix(int **matrix, int N) {
    for(int i = 0; i < N; i++)
        free(matrix[i]);
    free(matrix);
}

int **convolution(int *input, int **matrix) {
    int **output;
    /*
    Uint8 *base = (Uint8*)input->pixels;
    int pitch = input->pitch;
    int average;

    for (int y = 0; y < input->h; y++) {
        Uint8 *row = base + y * pitch; //row modifies input, because it points to it
        for (int x = 0; x < input->w; x++) {
            Uint8 r = row[x*4 + 0];
            Uint8 g = row[x*4 + 1];
            Uint8 b = row[x*4 + 2];
            Uint8 a = row[x*4 + 3];

            average = (r+g+b)/3;

            row[x*4 + 0] = average;
            row[x*4 + 1] = average;
            row[x*4 + 2] = average;
            row[x*4 + 3] = a;
        }
    }
    */
    return output;
}

int main(void)
{
    create_window();

    SDL_Surface *rgba = create_rgba("../src/dog.png");

    SDL_Window *win = SDL_CreateWindow(
        "png pixels",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        rgba->w, rgba->h,
        0
    );
    if (!win) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_FreeSurface(rgba);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(win);

    //ensure rgba is locked, unless we don't know the behavior
    if (SDL_MUSTLOCK(rgba)) {
        if (SDL_LockSurface(rgba) != 0) {
            printf("SDL_LockSurface error: %s\n", SDL_GetError());
            SDL_DestroyWindow(win);
            SDL_FreeSurface(rgba);
            IMG_Quit();
            SDL_Quit();
            return 1;
        }
    }

    Uint8 *base = (Uint8*)rgba->pixels;
    int pitch = rgba->pitch;
    int average;

    for (int y = 0; y < rgba->h; y++) {
        Uint8 *row = base + y * pitch; //row modifies rgba, because it points to it
        for (int x = 0; x < rgba->w; x++) {
            Uint8 r = row[x*4 + 0];
            Uint8 g = row[x*4 + 1];
            Uint8 b = row[x*4 + 2];
            Uint8 a = row[x*4 + 3];

            average = (r+g+b)/3;

            row[x*4 + 0] = average;
            row[x*4 + 1] = average;
            row[x*4 + 2] = average;
            row[x*4 + 3] = a;
        }
    }

    if (SDL_MUSTLOCK(rgba)) SDL_UnlockSurface(rgba); //like a "free()" because we don't need it anymore

    SDL_BlitSurface(rgba, NULL, screen, NULL);
    SDL_UpdateWindowSurface(win);

    printf("Hello World!");
    fflush(stdout);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }
        SDL_Delay(5);
    }

    int **test = convert_surface_to_int(rgba);
    free_matrix(test, rgba->h);

    SDL_FreeSurface(rgba);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
