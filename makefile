
CBP_prog: main.c
	gcc -o cbp main.c -lSDL2_ttf `sdl2-config --cflags --libs` -lm
