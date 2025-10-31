#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <locating.h>
#include <stdio.h>
#include <stdlib.h>

enum { BLACK = 0, WHITE = 255 };

uint8_t get_pixel(SDL_Surface *img, int x, int y) {
  if (x < 0 || x >= img->w || y < 0 || y >= img->h) {
    return 0;
  }

  // position of the pixel
  int bytes_per_pixel = img->format->BytesPerPixel;
  int offset = (y * img->pitch) + (x * bytes_per_pixel);

  uint8_t *pixel_address = (Uint8 *)img->pixels + offset;

  return *pixel_address;
}

// useful for flood fill
void set_pixel(SDL_Surface *img, int x, int y, uint8_t value) {
  if (x < 0 || x >= img->w || y < 0 || y >= img->h) {
    return;
  }

  int bytes_per_pixel = img->format->BytesPerPixel;
  int offset = (y * img->pitch) + (x * bytes_per_pixel);

  uint8_t *pixel_address = (uint8_t *)img->pixels + offset;
  *pixel_address = value;
}

void save_image(SDL_Surface *src, SDL_Rect zone, const char *filename) {
  SDL_Surface *img = SDL_CreateRGBSurface(0, zone.w, zone.h, 8, 0, 0, 0, 0);
  SDL_BlitSurface(src, &zone, img, NULL);
  IMG_SavePNG(img, filename);
  SDL_FreeSurface(img);
}

// proj_x and proj_y give the number of black pixels in a line and a column
// zones where there is a hight value indicates wether or not there's texte or
// grid In the contrary, low values indicates gaps ect...
void compute_projections(SDL_Surface *img, int *proj_x, int *proj_y) {
  for (int x = 0; x < img->w; x++) {
    for (int y = 0; y < img->h; y++) {
      if (get_pixel(img, x, y) == BLACK) {
        proj_x[x]++;
      }
    }
  }
  for (int y = 0; y < img->h; y++) {
    for (int x = 0; x < img->w; x++) {
      if (get_pixel(img, x, y) == BLACK) {
        proj_y[y]++;
      }
    }
  }
}

SDL_Rect detect_grid(SDL_Surface *img, int *proj_x, int *proj_y) {
  int treshold_x = img->h / 5; // if 20% of the line is black
  int treshold_y = img->w / 5;

  int left = -1;
  int right = -1;
  int top = -1;
  int bottom = -1;

  for (int x = 0; x < img->w; x++) {
    if (proj_x[x] > treshold_x && left == -1) {
      left = x;
    }
    if (proj_x[x] > treshold_x) {
      right = x;
    }
  }

  for (int y = 0; y < img->h; y++) {
    if (proj_y[y] > treshold_y && top == -1) {
      top = y;
    }
    if (proj_y[y] > treshold_y) {
      bottom = y;
    }
  }

  SDL_Rect grid = {left, top, right - left + 1, bottom - top + 1};
  return grid;
}

void flood_fill(SDL_Surface *img, int x, int y, struct bounding_box *bb) {
  if (x < 0 || y < 0 || x >= img->w || y >= img->h) {
    return;
  }
  if (get_pixel(img, x, y) != BLACK) {
    return;
  }

  // Mark as visited
  set_pixel(img, x, y, WHITE);

  if (x < bb->x_min) {
    bb->x_min = x;
  }
  if (y < bb->y_min) {
    bb->y_min = y;
  }
  if (x > bb->x_max) {
    bb->x_max = x;
  }
  if (y > bb->y_max) {
    bb->y_max = y;
  }

  flood_fill(img, x + 1, y, bb);
  flood_fill(img, x - 1, y, bb);
  flood_fill(img, x, y + 1, bb);
  flood_fill(img, x, y - 1, bb);
}

// Extracts each character as a sub-image
void extract_characters(SDL_Surface *zone, const char *folder) {
  int count = 0;
  for (int y = 0; y < zone->h; y++) {
    for (int x = 0; x < zone->w; x++) {
      if (get_pixel(zone, x, y) == BLACK) {
        struct bounding_box bb = {x, y, x, y};
        flood_fill(zone, x, y, &bb);
        SDL_Rect letter = {bb.x_min, bb.y_min, bb.x_max - bb.x_min + 1,
                           bb.y_max - bb.y_min + 1};

        char filename[128];
        (void)sprintf(filename, "%s/char_%03d.png", folder, count++);
        save_image(zone, letter, filename);
      }
    }
  }
  printf("Extracted %d character(s)\n", count);
}
