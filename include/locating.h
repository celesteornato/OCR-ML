#ifndef GRAYSCALE_H
#define GRAYSCALE_H
#include <SDL2/SDL_surface.h>

Uint8_t get_pixel(SDL_Surface *img, int x, int y);
void set_pixel(SDL_Surface *img, int x, int y, Uint8 value);
void save_image(SDL_Surface *src, SDL_Rect zone, const char *filename);

//projX and projY give the number of black pixels in a line and a column
//zones where there is a hight value indicates wether or not there's texte or grid
//In the contrary, low values indicates gaps ect...
void compute_projections(SDL_Surface *img, int *projX, int *projY);

SDL_Rect detect_grid(SDL_Surface *img, int *projX, int *projY);

typedef struct {int x_min, y_min, x_max, y_max;} BoundingBox;

void flood_fill(SDL_Surface *img, int x, int y, BoundingBox *bb);
void extract_characters(SDL_Surface *zone, const char *folder);




#endif