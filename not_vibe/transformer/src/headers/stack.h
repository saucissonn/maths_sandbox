#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

typedef struct {
    int x;
    int y;
} Coord;

typedef struct {
    Coord *data;
    int size;
    int capacity;
} Stack;

Stack *stack_create();
void stack_free(Stack *s);
int stack_push(Stack *s, int x, int y);
int stack_pop(Stack *s, Coord *out);
int stack_contains(Stack *s, int x, int y);

#endif