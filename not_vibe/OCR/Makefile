CC = gcc
PKGS = sdl3 SDL3_image

SAN = -fsanitize=address -fno-omit-frame-pointer
CFLAGS  = -g $(SAN) $(shell pkg-config --cflags $(PKGS))
LDFLAGS = $(SAN) $(shell pkg-config --libs $(PKGS))

n: main.c
	$(CC) main.c -o n $(CFLAGS) $(LDFLAGS)

run: n
	./n

clean:
	rm -f n
