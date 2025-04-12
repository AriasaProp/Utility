#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <ctime>

#include "sorting/sorter.hpp"

static void run_sort (const char *, const void *, size_t, void (*)(void*, size_t, size_t, int(*)(const void*, const void*)));
static int data_compare (const void*,const void*);
static int sorting_proof (const void *, size_t);

static void generate_random_data(void *dat, size_t size) {
	srand((unsigned int)time(NULL));
	float *ret = (float*) dat;
	for (size_t i = 0; i < size; ++i) {
		ret[i] = (float)((double)rand() / (double)RAND_MAX / 2.0) + 0.5f;
	}
}


void sorting_test() {
	unsigned long data_size = 10;
	void *data_r = malloc (data_size * sizeof(float));
	generate_random_data(data_r, data_size);
	printf(
		"Sorting Test! \n"
		"|           Name           ||         time         || \n"
		"|--------------------------||----------------------|| \n"
	);
	
	run_sort("Qsort_builtin", data_r, data_size, qsort);
	run_sort("Insertion", data_r, data_size, insertion_sort);
	run_sort("Bubble", data_r, data_size, bubble_sort);
	run_sort("Selection", data_r, data_size, selection_sort);
	run_sort("Merge", data_r, data_size, merge_sort);

	free (data_r);
}

static void run_sort (const char *name, const void *data, size_t sz, void (*s)(void*, size_t, size_t, int(*cmp)(const void*, const void*))) {
	clock_t c = clock();
	void *temp_data = malloc (sz * sizeof(float));
	memcpy(temp_data, data, sz * sizeof(float));
	s(temp_data, sz, sizeof(float), data_compare);
	int isSorted = sorting_proof(temp_data, sz);
	free (temp_data);
	unsigned long t = clock() - c; // clock
	
	printf("| %24s || %014lu clock ||%c \n", name, t, isSorted?'Y':'N');
}

static int data_compare(const void* a, const void* b) {
  float arg1 = *(const float*)a;
  float arg2 = *(const float*)b;
  return (arg1 < arg2) * -1 + (arg1 > arg2) * 1;
}
static int sorting_proof(const void *dat, size_t l) {
	float *ret = (float*) dat;
	for (size_t i = 1; i < l; ++i) {
		if (ret[i - 1] > ret[i]) return 0;
	}
	return 1;
}

/*
#include <SDL2/SDL.h>
struct App {
	SDL_Renderer *r;
	SDL_Window *w;
	int screen_width, screen_height;
};

static void scale_value(float, int*);

int main(int argc, char *argv[]) {
	// init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL failed to init %s\n", SDL_GetError());
		goto end;
	}
	struct App app;
	app.w = SDL_CreateWindow("SDL Sorting", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN);
	if (!app.w) {
		printf("Failed to make sdl window and renderer: %s\n", SDL_GetError());
		goto end_sdl;
	}
	app.r = SDL_CreateRenderer(app.w, -1, SDL_RENDERER_ACCELERATED);
	if (!app.r) {
		printf("Failed to make sdl window and renderer: %s\n", SDL_GetError());
		goto end_window_sdl;
	}
	SDL_GetWindowSize(app.w, &app.screen_width, &app.screen_height);
	SDL_Event e;
	
	// set data random
	size_t data_size = app.screen_height / 5 * 4;
	void *data_r = malloc (data_size * sizeof(float));
	generate_random_data(data_r, data_size);
	

	// on start
	int running = 1;
	while (running) {
		// get input
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONUP:
				break;
			default:
				break;
			}
		}
		
		for (size_t i = 0; i < data_size; ++i) {
			int rgb[3];
			scale_value(((float*)data_r)[i], rgb);
			SDL_SetRenderDrawColor(app.r, rgb[0], rgb[1], rgb[2], 255);

			SDL_Rect rect = {app.screen_width / 4, app.screen_height / 40 + i, app.screen_width / 2, 1 };
			SDL_RenderFillRect(app.r, &rect);
		}
		
		SDL_RenderPresent(app.r);
	}
	free (data_r);
	
	SDL_DestroyRenderer(app.r);
end_window_sdl:
	SDL_DestroyWindow(app.w);
end_sdl:
	SDL_Quit();
end:
	return 0;
}
static double inline clamp_color(double d) {
  return (d > 255.0) ? 255.0 : ((d > 0.0) * d);
}

static void scale_value(float i, int* rgb) {
	rgb[0] = (int)clamp_color(85.0 * (6.0 * i - 2.0) * (6.0 * i - 4.0));
	rgb[1] = (int)clamp_color(255.0 * i * (8.0 - 12.0 * i));
	rgb[2] = (int)clamp_color(255.0 * (12.0 * i - 4.0) * (1.0 - i));
}
*/