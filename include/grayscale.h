#ifndef GRAYSCALE_H_
#define GRAYSCALE_H_
#include <SDL2/SDL_surface.h>

/* Returns a grayscale copy of the surface src */
SDL_Surface *grayscale(SDL_Surface *src);

/* Compute the threshold of gray off the image by computing the noise of the
 * picture (Otsu method) based on the shape of the image histogram
 * returns a threshold usesful for next function, et oui Virgile */
uint8_t get_threshold(const SDL_Surface *gray);

/* Return a black and white copy of the surface, according to the threshold */
SDL_Surface *apply_threshold(SDL_Surface *src, uint8_t threshold);

#endif // GRAYSCALE_H_
