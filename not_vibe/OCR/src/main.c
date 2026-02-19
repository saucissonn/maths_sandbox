#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>
#include <time.h>

//open an image and display it on a window

SDL_Surface *img;
SDL_Window *win;
SDL_Surface *rgba;
int width;
int height;

struct layer {
    char *name;
    int previous_size;
    int current_size;  //16
    double *weights;   //matrix (output_size × input_size)
    double *biases;    //output_size
    double *output;    //activations
    double *delta;     //gradients
};

SDL_Surface * create_rgba(char * image) {
    //give rgba of an image to extract info of each pixel
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

int create_window(char * image) {
    //init, create window and rgba
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    rgba = create_rgba(image);

    width = rgba->w;
    height = rgba->h;

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
    for (int y = 0; y < height; y++) {
        Uint8 *row = base + y * pitch; //row modifies input, because it points to it
        for (int x = 0; x < width; x++) {
            Uint8 r = row[x*4 + 0];
            Uint8 g = row[x*4 + 1];
            Uint8 b = row[x*4 + 2];
            Uint8 a = row[x*4 + 3];

            average = 255 - (r+g+b)/3; //invert black and white

            row[x*4 + 0] = average;
            row[x*4 + 1] = average;
            row[x*4 + 2] = average;
            row[x*4 + 3] = a;
        }
    }
    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input); //like a "free()" because we don't need it anymore
}

double **convert_surface_to_double(SDL_Surface * input) {
    double **output = malloc((input->h)*sizeof(*output));
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

            output[y][x] = (double)average;
        }
    }
    return output;
}

void free_matrix(double **matrix, int N) {
    for(int i = 0; i < N; i++)
        free(matrix[i]);
    free(matrix);
}

void print_matrix(double **input, int width, int height) {
    double v = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            v = input[y][x];
            if (v < 10) {
                printf("%.1lf  ", v);
            }
            else {
                if (v < 100) {
                    printf("%.1lf ", v);
                }
                else {
                    printf("%.1lf", v);
                }
            }
        }
        printf("\n");
    } 
}

int **convolution(int *input, int **matrix) {
    int **output;
    return output;
}

void relu(struct layer l) {
    int cs = l.current_size;
    for (int i = 0; i < cs; i++) {
        if ((l.output)[i] < 0) {
            (l.output)[i] = 0;
        }
    }
}

double rand_uniform() {
    return 2.0 * rand() / (double)RAND_MAX - 1.0;
}

struct layer init_layer(char * name, int previous_size, int current_size) {
    struct layer res;
    res.previous_size = previous_size;
    res.current_size = current_size;
    res.name = name;
    if (name[0] != 'i') {
        res.weights = malloc(previous_size * current_size * sizeof(double));
        double scale = sqrt(6.0 / (previous_size + current_size));
        for (int i = 0; i < previous_size * current_size; i++)
        {
            res.weights[i] = rand_uniform() * scale;
        }

        res.biases = malloc(current_size * sizeof(double));
        for (int i = 0; i < current_size; i++) {
            res.biases[i] = 0.0;
        }

        res.output = malloc(current_size * sizeof(double));
        res.delta = malloc(current_size * sizeof(double));
    }
    else {
        res.weights = NULL;
        res.biases = NULL;
        res.output = malloc(current_size * sizeof(double));
        res.delta = NULL;
    }
    return res;
}

void free_layer(struct layer l) {
        if (l.weights)
            free(l.weights);
        if (l.biases)
            free(l.biases);
        if (l.output)
            free(l.output);
        if (l.delta)
            free(l.delta);
}

void print_outputs(struct layer l) {
    int cs = l.current_size;
    for (int i = 0; i < cs; i++) {
        printf("%lf\n", (l.output)[i]);
    } 
}

void soft_max(struct layer l) {
    int cs = l.current_size;
    double max = l.output[0];
    for (int i = 0; i < cs; i++) {
        if (max < (l.output)[i]) {
            max = (l.output)[i];
        }
    }
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum += exp((l.output)[i] - max);
    }
    for (int i = 0; i < cs; i++) {
        (l.output)[i] = exp((l.output)[i] - max)/sum;
    }
}

void sigmoid(struct layer l) {
    int cs = l.current_size;
    double one = 1;
    for (int i = 0; i < cs; i++) {
        (l.output)[i] = one/(one + exp(-(l.output)[i]));
    }
}

void forward(struct layer prev, struct layer curr) {
    int cs = curr.current_size;
    int ps = curr.previous_size;
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum = -(curr.biases)[i];
        for (int j = 0; j < ps; j++) {
            sum += ((prev.output)[j])*((curr.weights)[i*cs + j]);
        }
        (curr.output)[i] = sum;
    }
}

int main(void)
{
    srand(time(NULL));
    create_window("../src/dog.png"); //also create rgba which is the raw image

    SDL_Surface *screen = SDL_GetWindowSurface(win);

    display_SDL_matrix(rgba, width, height);
    SDL_BlitSurface(rgba, NULL, screen, NULL);
    SDL_UpdateWindowSurface(win);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }
        SDL_Delay(5);
    }

    double **raw_data = convert_surface_to_double(rgba);
    printf("ok\n");

    int nb_input_neurons = width*height;
    int nb_hidden_neurons = 16;
    int nb_output_neurons = 10;

    struct layer input_layer = init_layer("input", nb_input_neurons, nb_input_neurons);

    int c = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            (input_layer.output)[c] = raw_data[y][x];
            c++;
        }
    } 

    struct layer hidden_layer1 = init_layer("hidden", nb_input_neurons, nb_hidden_neurons);
    struct layer output_layer = init_layer("output", nb_hidden_neurons, nb_output_neurons);

    forward(input_layer, hidden_layer1);
    relu(hidden_layer1);
    forward(hidden_layer1, output_layer);
    soft_max(output_layer);

    print_outputs(output_layer);

    free_layer(input_layer);
    free_layer(hidden_layer1);
    free_layer(output_layer);

    //print_matrix(raw_data, width, height);
    free_matrix(raw_data, height);

    SDL_FreeSurface(rgba);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
