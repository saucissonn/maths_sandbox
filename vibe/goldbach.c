#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif

static void write_i32(int32_t x) {
    fwrite(&x, sizeof(x), 1, stdout);
}

int main(void)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    int n;
    if (scanf("%d", &n) != 1) return 1;

    int *premiers = malloc((n + 1) * sizeof(int));
    bool *is_prime = calloc((size_t)n + 1, sizeof(bool));
    if (!premiers || !is_prime) return 1;

    int count = 0;
    if (n >= 2) {
        premiers[count++] = 2;
        is_prime[2] = true;
    }

    for (int i = 3; i <= n; i += 2) {
        bool est_premier = true;
        for (int j = 0; j < count; j++) {
            int p = premiers[j];
            if (p * p > i) break;
            if (i % p == 0) { est_premier = false; break; }
        }
        if (est_premier) {
            premiers[count++] = i;
            is_prime[i] = true;
        }
    }

    // 1er passage : compter TOUTES les combinaisons (p <= q) pour chaque i pair
    int32_t triplets = 0;
    for (int i = 4; i <= n; i += 2) {
        for (int j = 0; j < count; j++) {
            int p = premiers[j];
            if (p > i / 2) break;           // p <= q pour éviter les doublons
            int q = i - p;
            if (q >= 0 && q <= n && is_prime[q]) triplets++;
        }
    }

    // écriture binaire : nb de triplets, puis triplets*(p,q,i)
    write_i32(triplets);

    for (int i = 4; i <= n; i += 2) {
        for (int j = 0; j < count; j++) {
            int p = premiers[j];
            if (p > i / 2) break;
            int q = i - p;
            if (q >= 0 && q <= n && is_prime[q]) {
                write_i32((int32_t)p);
                write_i32((int32_t)q);
                write_i32((int32_t)i);
            }
        }
    }

    free(is_prime);
    free(premiers);
    return 0;
}
