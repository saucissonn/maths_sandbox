#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

// Génère un float uniforme dans [a, b]
float rand_uniform_token(float a, float b) {
    return a + (b - a) * ((float)rand() / (float)RAND_MAX);
}

// Initialisation Xavier / Glorot
void init_xavier(float *E, int vocab_size, int d) {
    int d_in = vocab_size;
    int d_out = d;

    float limit = sqrtf(6.0f / (d_in + d_out));

    for (int i = 0; i < vocab_size * d; i++) {
        E[i] = rand_uniform_token(-limit, limit);
    }
}

int main() {
    srand(time(NULL));

    int vocab_size = 7972;
    int dmodel = 512;

    float *E = malloc(sizeof(float) * vocab_size * d);

    init_xavier(E, vocab_size, d);

    // exemple: afficher 5 valeurs
    for (int i = 0; i < 512; i++) {
        printf("%f\n", E[i]);
    }

    free(E);
    return 0;
}