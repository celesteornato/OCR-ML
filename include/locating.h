#ifndef LOCATING_H
#define LOCATING_H
#include <SDL2/SDL_surface.h>

struct bounding_box {int x_min; int y_min; int x_max; int y_max;};

uint8_t get_pixel(SDL_Surface *img, int x, int y);
void set_pixel(SDL_Surface *img, int x, int y, uint8_t value);
void save_image(SDL_Surface *src, SDL_Rect zone, const char *filename);

//projX and projY give the number of black pixels in a line and a column
//zones where there is a hight value indicates wether or not there's texte or grid
//In the contrary, low values indicates gaps ect...
void compute_projections(SDL_Surface *img, int *proj_x, int *proj_y);


SDL_Rect detect_grid(SDL_Surface *img, int *proj_x, int *proj_y);

void flood_fill(SDL_Surface *img, int x, int y, struct bounding_box *bb);
void extract_characters(SDL_Surface *zone, const char *folder);

#endif
