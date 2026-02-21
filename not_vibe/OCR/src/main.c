#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // nix-shell -p SDL2 SDL2_image pkg-config (command for nixos)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>

//open an image and display it on a window

SDL_Surface *img;
SDL_Window *win;
SDL_Surface *rgba;
int width = 28;
int height = 28;
int expected;
double loss;
double **raw_data;
int random_x = 5;
DIR *dir;
int c_steps = 0;
double learning_coeff = 0.001;
int nb_hidden_neurons = 256;
int nb_output_neurons = 10;

struct layer {
    char *name;
    int previous_size;
    int current_size;  //16
    double *weights;   //matrix (output_size × input_size)
    double *biases;    //output_size
    double *output;    //activations
    double *delta;     //gradients
    double *z;
};

struct layer input_layer;
struct layer hidden_layer1;
struct layer hidden_layer2;
struct layer output_layer;

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
    int average;
    Uint8 *row;
    double v;
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

            v = (r+g+b)/765.0;

            output[y][x] = v;
        }
    }
    if (SDL_MUSTLOCK(input)) SDL_UnlockSurface(input); //like a "free()" because we don't need it anymore
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
            v = input[y][x] * 255.0;
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

int select_random_file() {
    random_x = rand() % nb_output_neurons;
    expected = random_x;

    char path2[256];
    snprintf(path2, sizeof(path2), "../src/training/%d", expected);

    DIR *dir = opendir(path2);
    if (!dir) return 1;

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL)
        if (entry->d_type == DT_REG) count++;

    if (count == 0) { closedir(dir); return 1; }

    int target = rand() % count;
    rewinddir(dir);

    int i = 0;
    entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (i == target) break;
            i++;
        }
    }
    closedir(dir);
    if (!entry) return 1;

    char path[256];
    snprintf(path, sizeof(path), "../src/training/%d/%s", expected, entry->d_name);


    if (create_window(path) != 0) return 1;
    /*
    if (rgba->w != width || rgba->h != height) {
        printf("SIZE MISMATCH: got %dx%d expected %dx%d\n", rgba->w, rgba->h, width, height);
        return 1;
    }
    */

    return 0;
}

int **convolution(int *input, int **matrix) {
    int **output;
    (void)input;
    (void)matrix;
    return output;
}

void relu(struct layer *l) {
    int cs = l->current_size;
    for (int i = 0; i < cs; i++) {
        if (l->output[i] < 0) {
            l->output[i] = 0;
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
            res.biases[i] = 0.01;
        }

        res.output = malloc(current_size * sizeof(double));
        res.delta = malloc(current_size * sizeof(double));
        res.z = malloc(current_size * sizeof(double));
    }
    else {
        res.weights = NULL;
        res.biases = NULL;
        res.output = malloc(current_size * sizeof(double));
        res.delta = NULL;
        res.z = NULL;
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
        if (l.z)
            free(l.z);
}

void print_outputs(struct layer l) {
    int cs = l.current_size;
    for (int i = 0; i < cs; i++) {
        printf("%d: %lf\n", i, (l.output)[i]);
    }
}

int index_max_output(struct layer l) {
    int cs = l.current_size;
    int maxi = 0;
    for (int i = 0; i < cs; i++) {
        if (l.output[maxi] < l.output[i])
            maxi = i;
    }
    return maxi;
}

void soft_max(struct layer *l) {
    int cs = l->current_size;
    double max = l->output[0];
    for (int i = 1; i < cs; i++) {
        if (max < l->output[i]) {
            max = l->output[i];
        }
    }
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum += exp(l->output[i] - max);
    }
    for (int i = 0; i < cs; i++) {
        l->output[i] = exp(l->output[i] - max) / sum;
    }
}

void sigmoid(struct layer *l) {
    int cs = l->current_size;
    for (int i = 0; i < cs; i++) {
        l->output[i] = 1./(1. + exp(-(l->output)[i]));
    }
}

void forward(struct layer *prev, struct layer *curr) {
    int cs = curr->current_size;
    int ps = curr->previous_size;
    double sum = 0;
    for (int i = 0; i < cs; i++) {
        sum = curr->biases[i];
        for (int j = 0; j < ps; j++) {
            sum += prev->output[j] * curr->weights[i*ps + j];
        }
        curr->z[i] = sum;
        curr->output[i] = sum;
    }
}

double relu_prime(double v) {
    if (v <= 0)
        return 0;
    return 1.0;
}

void backward(struct layer *curr, struct layer *prev) {
    int cs = curr->current_size;
    int ps = curr->previous_size;

    if (!prev->delta) return;

    double s = 0;
    for (int j = 0; j < ps; j++) {
        s = 0.0;
        for (int i = 0; i < cs; i++) {
            s += curr->weights[i*ps + j] * curr->delta[i];
        }
        if (prev->z) {
            prev->delta[j] = s * relu_prime(prev->z[j]);
        } else {
            prev->delta[j] = s;
        }
    }
}

void update_SGD(struct layer *curr, struct layer *prev) {
    int cs = curr->current_size;
    int ps = curr->previous_size;

    for (int i = 0; i < cs; i++) {
        for (int j = 0; j < ps; j++) {
            curr->weights[i*ps + j] -= learning_coeff * curr->delta[i] * prev->output[j];
        }
        curr->biases[i] -= learning_coeff * curr->delta[i];
    }
}

void f_and_b() {
    forward(&input_layer, &hidden_layer1);
    relu(&hidden_layer1);
    forward(&hidden_layer1, &hidden_layer2);
    relu(&hidden_layer2);
    forward(&hidden_layer2, &output_layer);
    soft_max(&output_layer);

    double p = output_layer.output[expected];
    if (p < 1e-15) p = 1e-15;
    loss = -log(p);
    int idx_max_output = index_max_output(output_layer);
    int result = 0;
    if (idx_max_output == expected)
        result = 1;
    printf("loss: %f, value: %f, steps: %d, expected: %d, get: %d, result: %d\n", loss, output_layer.output[expected], c_steps++, expected, idx_max_output, result);

    //print_outputs(output_layer);

    for (int i = 0; i < output_layer.current_size; i++) {
        output_layer.delta[i] = output_layer.output[i];
    }
    output_layer.delta[expected] -= 1.0; //cross entropy

    backward(&output_layer, &hidden_layer2);
    backward(&hidden_layer2, &hidden_layer1);
    backward(&hidden_layer1, &input_layer);

    update_SGD(&output_layer,  &hidden_layer2);
    update_SGD(&hidden_layer2, &hidden_layer1);
    update_SGD(&hidden_layer1, &input_layer);
}

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

        double **raw_data = convert_surface_to_double(rgba);

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
        f_and_b();

        free_matrix(raw_data, height);
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
