#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/data.h"
#include "headers/globals.h"
#include "headers/nn.h"

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

int select_random_file(void) {
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

    /*
    if (rgba->w != width || rgba->h != height) {
        printf("SIZE MISMATCH: got %dx%d expected %dx%d\n", rgba->w, rgba->h, width, height);
        return 1;
    }
    */

    return 0;
}

static int write_u32(FILE *f, uint32_t v){ return fwrite(&v, sizeof(v), 1, f) == 1; }
static int write_u64(FILE *f, uint64_t v){ return fwrite(&v, sizeof(v), 1, f) == 1; }

static int read_u32(FILE *f, uint32_t *v){ return fread(v, sizeof(*v), 1, f) == 1; }
static int read_u64(FILE *f, uint64_t *v){ return fread(v, sizeof(*v), 1, f) == 1; }

int layer_save(FILE *f, const struct layer *l)
{
    const uint32_t magic = 0x4C415952; // LAYR
    const uint32_t ver   = 1;

    if (!write_u32(f, magic) || !write_u32(f, ver)) return 0;

    uint32_t name_len = l->name ? (uint32_t)strlen(l->name) : 0;
    if (!write_u32(f, name_len)) return 0;
    if (name_len && fwrite(l->name, 1, name_len, f) != name_len) return 0;

    if (!write_u32(f, (uint32_t)l->previous_size)) return 0;
    if (!write_u32(f, (uint32_t)l->current_size)) return 0;

    uint64_t wcount = (uint64_t)l->previous_size * (uint64_t)l->current_size;
    uint64_t bcount = (uint64_t)l->current_size;

    if (!write_u64(f, wcount) || !write_u64(f, bcount)) return 0;

    if (fwrite(l->weights, sizeof(double), (size_t)wcount, f) != (size_t)wcount) return 0;
    if (fwrite(l->biases,  sizeof(double), (size_t)bcount, f) != (size_t)bcount) return 0;

    return 1;
}

int layer_load(FILE *f, struct layer *l)
{
    memset(l, 0, sizeof(*l));

    uint32_t magic=0, ver=0;
    if (!read_u32(f, &magic) || !read_u32(f, &ver)) return 0;
    if (magic != 0x4C415952 || ver != 1) return 0;

    uint32_t name_len=0;
    if (!read_u32(f, &name_len)) return 0;

    if (name_len) {
        l->name = malloc(name_len + 1);
        if (!l->name) return 0;
        if (fread(l->name, 1, name_len, f) != name_len) return 0;
        l->name[name_len] = '\0';
    }

    uint32_t ps=0, cs=0;
    if (!read_u32(f, &ps) || !read_u32(f, &cs)) return 0;

    l->previous_size = (int)ps;
    l->current_size  = (int)cs;

    uint64_t wcount=0, bcount=0;
    if (!read_u64(f, &wcount) || !read_u64(f, &bcount)) return 0;

    if (wcount != (uint64_t)ps * (uint64_t)cs) return 0;
    if (bcount != (uint64_t)cs) return 0;

    l->weights = malloc(wcount * sizeof(double));
    l->biases  = malloc(bcount * sizeof(double));
    l->output  = calloc(cs, sizeof(double));
    l->delta   = calloc(cs, sizeof(double));
    l->z       = calloc(cs, sizeof(double));

    if (!l->weights || !l->biases || !l->output || !l->delta || !l->z) return 0;

    if (fread(l->weights, sizeof(double), (size_t)wcount, f) != (size_t)wcount) return 0;
    if (fread(l->biases,  sizeof(double), (size_t)bcount, f) != (size_t)bcount) return 0;

    return 1;
}

void layer_free(struct layer *l)
{
    if (!l) return;

    free(l->weights);
    free(l->biases);
    free(l->output);
    free(l->delta);
    free(l->z);
    free(l->name);

    memset(l, 0, sizeof(*l));
}
