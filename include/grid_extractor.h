#ifndef GRID_EXTRACTOR_H
#define GRID_EXTRACTOR_H

#include <SDL2/SDL.h>

#define LINE_THRESHOLD 100

struct grid_bounds {
    int top;
    int bottom;
    int left;
    int right;
};

void get_h_projection(SDL_Surface *img, int *proj);

void get_w_projection(SDL_Surface *img, int *proj);

void find_bounds(int *proj, int length, int *start, int *end);

struct grid_bounds get_grid(SDL_Surface *img);

void save_image(SDL_Surface *img, SDL_Rect rect, const char *filename);

void extract_cells(SDL_Surface *img, struct grid_bounds g, int rows, int cols, const char *prefix);

void extract_list(SDL_Surface *img, struct grid_bounds g, const char *filename);

//I have to change the main file
#endif
