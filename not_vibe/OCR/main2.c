#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL2/SDL.h> // nix-shell -p SDL2 pkg-config (command for nixos)

int main()
{
    FILE *file;
    int width = 0;
    int height = 0;
    file = fopen("dog2.ppm", "r");
    size_t len = 0;
    char *line = NULL;
    int read;

    while ((read = getline(&line, &len, file)) > -1) {
        if (isdigit(line[0]))
            break;
    }

    //take w and h values
    sscanf(line, "%d %d", &width, &height);

    //skip next line
    getline(&line, &len, file);

    if (line)
        free(line);


    //init window
    printf("Hello World!");
    SDL_Window * screen = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    SDL_Surface * surface = SDL_GetWindowSurface(screen);

    Uint32 color = 0;

    SDL_Rect pixel = (SDL_Rect){0, 0, 1, 1};
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8 r, g, b;
            r = (char) getc(file);
            g = (char) getc(file);
            b = (char) getc(file);
            color = SDL_MapRGB(surface->format, r, g, b);
            pixel.x = x;
            pixel.y = y;
            SDL_FillRect(surface, &pixel, color); //with NULL it fills the whole screen with the color
        }
    }

    SDL_UpdateWindowSurface(screen);
    SDL_Delay(3000);

    return 0;
}
