#include <stdio.h>
#include <stdlib.h>

#include "headers/segment.h"

static int clampi(int x, int lo, int hi) { return (x < lo) ? lo : (x > hi) ? hi : x; }

static Line *segment_lines(double **img, int H, int W,
                           int *out_n,
                           int row_ink_thr,
                           int min_gap,
                           int min_height)
{
    *out_n = 0;
    int *rowsum = (int*)calloc((size_t)H, sizeof(int));
    if (!rowsum) return NULL;

    for (int y = 0; y < H; y++) {
        int s = 0;
        for (int x = 0; x < W; x++) s += (img[y][x] > 0);
        rowsum[y] = s;
    }

    int cap = 16, n = 0;
    Line *lines = (Line*)malloc((size_t)cap * sizeof(Line));
    if (!lines) { free(rowsum); return NULL; }

    int y = 0;
    while (y < H) {
        while (y < H && rowsum[y] <= row_ink_thr) y++;
        if (y >= H) break;

        int start = y;

        int gaprun = 0;
        while (y < H) {
            if (rowsum[y] <= row_ink_thr) gaprun++;
            else gaprun = 0;

            if (gaprun >= min_gap) {
                int end = y - gaprun;
                int h = end - start + 1;
                if (h >= min_height) {
                    if (n == cap) {
                        cap *= 2;
                        Line *tmp = (Line*)realloc(lines, (size_t)cap * sizeof(Line));
                        if (!tmp) {
                            free(rowsum); free(lines);
                            return NULL;
                        }
                        lines = tmp;
                    }
                    lines[n++] = (Line){ start, end };
                }
                break;
            }
            y++;
        }

        if (y >= H) {
            int end = H - 1;
            int h = end - start + 1;
            if (h >= min_height) {
                if (n == cap) {
                    cap *= 2;
                    Line *tmp = (Line*)realloc(lines, (size_t)cap * sizeof(Line));
                    if (!tmp) {
                        free(rowsum);
                        free(lines); return NULL;
                    }
                    lines = tmp;
                }
                lines[n++] = (Line){ start, end };
            }
        }
    }

    free(rowsum);
    *out_n = n;
    return lines;
}

static Box make_box_from_xrange(double **img, int y0, int y1, int x0, int x1) {
    Box b;
    b.min_r = y1; b.max_r = y0;
    b.min_c = x0; b.max_c = x1;
    b.area = 0;

    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (img[y][x] > 0.5) {
                b.area++;
                if (y < b.min_r) b.min_r = y;
                if (y > b.max_r) b.max_r = y;
            }
        }
    }

    if (b.area == 0) { b.min_r = 0; b.max_r = -1; }
    return b;
}

static Box *segment_chars_in_line(double **img, int H, int W,
                                 Line line,
                                 int *out_n,
                                 int col_ink_thr,
                                 int min_gap,
                                 int min_width,
                                 int min_area)
{
    *out_n = 0;
    int y0 = clampi(line.y0, 0, H-1);
    int y1 = clampi(line.y1, 0, H-1);
    if (y1 < y0) return NULL;

    int *colsum = (int*)calloc((size_t)W, sizeof(int));
    if (!colsum) return NULL;

    for (int x = 0; x < W; x++) {
        int s = 0;
        for (int y = y0; y <= y1; y++) s += (img[y][x] > 0);
        colsum[x] = s;
    }

    int cap = 64, n = 0;
    Box *boxes = (Box*)malloc((size_t)cap * sizeof(Box));

    if (!boxes) {
        free(colsum);
        return NULL;
    }

    int x = 0;
    while (x < W) {
        while (x < W && colsum[x] <= col_ink_thr) x++;
        if (x >= W) break;

        int start = x;
        int gaprun = 0;

        while (x < W) {
            if (colsum[x] <= col_ink_thr) gaprun++;
            else gaprun = 0;

            if (gaprun >= min_gap) {
                int end = x - gaprun;
                int w = end - start + 1;
                if (w >= min_width) {
                    Box b = make_box_from_xrange(img, y0, y1, start, end);
                    if (b.area >= min_area && b.max_r >= b.min_r) {
                        if (n == cap) {
                            cap *= 2;
                            Box *tmp = (Box*)realloc(boxes, (size_t)cap * sizeof(Box));
                            if (!tmp) { free(colsum); free(boxes); return NULL; }
                            boxes = tmp;
                        }
                        boxes[n++] = b;
                    }
                }
                break;
            }
            x++;
        }

        if (x >= W) {
            int end = W - 1;
            int w = end - start + 1;
            if (w >= min_width) {
                Box b = make_box_from_xrange(img, y0, y1, start, end);
                if (b.area >= min_area && b.max_r >= b.min_r) {
                    if (n == cap) {
                        cap *= 2;
                        Box *tmp = (Box*)realloc(boxes, (size_t)cap * sizeof(Box));
                        if (!tmp) { free(colsum); free(boxes); return NULL; }
                        boxes = tmp;
                    }
                    boxes[n++] = b;
                }
            }
        }
    }

    free(colsum);
    *out_n = n;
    return boxes;
}

static Box pad_box(Box b, int H, int W, int pad) {
    b.min_r = clampi(b.min_r - pad, 0, H-1);
    b.min_c = clampi(b.min_c - pad, 0, W-1);
    b.max_r = clampi(b.max_r + pad, 0, H-1);
    b.max_c = clampi(b.max_c + pad, 0, W-1);
    return b;
}

static double *extract_square_from_box(double **img, Box b, int *S_out) {
    int h = b.max_r - b.min_r + 1;
    int w = b.max_c - b.min_c + 1;
    int S = (h > w) ? h : w;

    double *sq = (double*)calloc((size_t)S * (size_t)S, sizeof(double));
    if (!sq) return NULL;

    int off_r = (S - h) / 2;
    int off_c = (S - w) / 2;

    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            sq[(r + off_r)*S + (c + off_c)] = img[b.min_r + r][b.min_c + c];
        }
    }

    *S_out = S;
    return sq;
}

static double *resize_nn(const double *in, int S, int newS) {
    double *out = (double*)malloc((size_t)newS * (size_t)newS * sizeof(double));
    if (!out) return NULL;

    for (int r = 0; r < newS; r++) {
        for (int c = 0; c < newS; c++) {
            int src_r = (r * S) / newS;
            int src_c = (c * S) / newS;
            out[r*newS + c] = in[src_r*S + src_c];
        }
    }
    return out;
}

static void binarize(double *img, int n, double thr) {
    for (int i = 0; i < n; i++) img[i] = (img[i] > thr) ? 1.0 : 0.0;
}

static double *box_to_28x28(double **img, int H, int W, Box b, int pad_pixels, double thr) {
    b = pad_box(b, H, W, pad_pixels);

    int S = 0;
    double *sq = extract_square_from_box(img, b, &S);
    if (!sq) return NULL;

    double *out28 = resize_nn(sq, S, 28);
    free(sq);
    if (!out28) return NULL;

    binarize(out28, 28*28, thr);
    return out28;
}

static void print_28x28_ascii(const double *x) {
    for (int r = 0; r < 28; r++) {
        for (int c = 0; c < 28; c++) putchar(x[r*28 + c] > 0.5 ? '#' : '.');
        putchar('\n');
    }
}

void segment_and_print(double **data, int H, int W) {
    const int row_ink_thr = 2;
    const int line_min_gap = 2;
    const int line_min_h = 10;

    int nlines = 0;
    Line *lines = segment_lines(data, H, W, &nlines, row_ink_thr, line_min_gap, line_min_h);
    if (!lines) { printf("No lines\n"); return; }

    printf("Lines: %d\n", nlines);

    for (int li = 0; li < nlines; li++) {
        Line L = lines[li];
        printf("\n== LINE %d: y[%d..%d] ==\n", li, L.y0, L.y1);

        const int col_ink_thr = 0;
        const int char_min_gap = 1;
        const int char_min_w   = 1;
        const int char_min_area = 5;

        int nch = 0;
        Box *chars = segment_chars_in_line(data, H, W, L, &nch,
                                           col_ink_thr, char_min_gap, char_min_w, char_min_area);
        if (!chars) { printf("No chars\n"); continue; }

        printf("Chars: %d\n", nch);

        for (int i = 0; i < nch; i++) {
            Box b = chars[i];
            printf("char %d: r[%d..%d] c[%d..%d] area=%d\n",
                   i, b.min_r, b.max_r, b.min_c, b.max_c, b.area);

            double *x28 = box_to_28x28(data, H, W, b, 1, 0.5);
            if (x28) {
                print_28x28_ascii(x28);
                printf("\n");
                free(x28);
            }
        }

        free(chars);
    }

    free(lines);
}