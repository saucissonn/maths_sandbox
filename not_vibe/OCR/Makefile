CC = gcc

CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image)

LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image)



n: main.c

	$(CC) main.c -o n $(CFLAGS) $(LDFLAGS)
