#include <SDL2/SDL.h>
#include <grayscale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Surface *grayscale(SDL_Surface *src) {
  // Our approach will be to modify this surface
  SDL_Surface *gray = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0);
  if (!gray) {
    return NULL;
  }
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  // The RGB888 format is analogous to a uint32_t
  uint32_t *pixels = gray->pixels;

  for (int y = 0; y < gray->h; y++) {
    for (int x = 0; x < gray->w; x++) {
      uint32_t pixel = pixels[(y * gray->w) + x];

      // get color of the pixel with RGB code
      SDL_GetRGB(pixel, gray->format, &r, &g, &b);
      // compute the gray
      uint8_t val = (uint8_t)((0.299 * r) + (0.587 * g) + (0.114 * b));
      // write new pixel over old one in grey
      pixels[(y * gray->w) + x] = SDL_MapRGB(gray->format, val, val, val);
    }
  }
  return gray; // return new image in grey
}

uint8_t get_threshold(const SDL_Surface *gray) {
  uint64_t histogram[UINT8_MAX + 1];
  uint64_t pixel_count = (uint64_t)gray->h * (uint64_t)gray->w;
  uint64_t average = 0;

  uint32_t *pixels = (uint32_t *)gray->pixels; // all pixels of the gray picture
  for (int y = 0; y < gray->h; y++) {
    for (int x = 0; x < gray->w; x++) {
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;

      SDL_GetRGB(pixels[(y * gray->w) + x], gray->format, &r, &g, &b);
      // Because we take a grayscale image, r = g = b
      histogram[r]++;
    }
  }
  return UINT8_MAX/2;
}

SDL_Surface *apply_threshold(SDL_Surface *src, uint8_t threshold) {
  SDL_Surface *bnw = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888,
                                              0); // we create a new surface
  if (!bnw) {
    return NULL;
  }

  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint32_t *pixels = (uint32_t *)bnw->pixels; // same
  for (int y = 0; y < bnw->h; y++)            // going through all pixels
  {
    for (int x = 0; x < bnw->w; x++) {
      uint32_t pixel = pixels[(y * bnw->w) + x]; // get the pixel
      SDL_GetRGB(pixel, bnw->format, &r, &g,
                 &b); // get color of the pixel with RGB code here r == g == b
      if (r > threshold) {
        pixels[(y * bnw->w) + x] =
            SDL_MapRGB(bnw->format, 255, 255, 255); // in white
      } else {
        pixels[(y * bnw->w) + x] =
            SDL_MapRGB(bnw->format, 0, 0,
                       0); // write new pixel over old one in black
      }
    }
  }
  return bnw;
}
