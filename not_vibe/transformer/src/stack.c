#include <stdlib.h>

#include "headers/globals.h"
#include "headers/stack.h"

Stack *stack_create() {
    Stack *s = malloc(sizeof(Stack));
    if (!s) return NULL;

    s->data = malloc(sizeof(Coord) * INIT_CAP_STACK);
    if (!s->data) {
        free(s);
        return NULL;
    }

    s->size = 0;
    s->capacity = INIT_CAP_STACK;
    return s;
}

void stack_free(Stack *s) {
    if (!s) return;
    free(s->data);
    free(s);
}

int stack_push(Stack *s, int x, int y) {
    if (s->size >= s->capacity) {
        int new_cap = s->capacity * 2;
        Coord *tmp = realloc(s->data, sizeof(Coord) * new_cap);
        if (!tmp) return 0;

        s->data = tmp;
        s->capacity = new_cap;
    }

    s->data[s->size].x = x;
    s->data[s->size].y = y;
    s->size++;

    return 1;
}

int stack_pop(Stack *s, Coord *out) {
    if (s->size == 0) return 0;

    s->size--;
    if (out) *out = s->data[s->size];

    return 1;
}

int stack_contains(Stack *s, int x, int y) {
    for (int i = 0; i < s->size; i++) {
        if (s->data[i].x == x && s->data[i].y == y) {
            return 1;
        }
    }
    return 0;
}