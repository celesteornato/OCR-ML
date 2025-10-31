#include <SDL2/SDL.h>
#include <assert.h>
#include <getopt.h>
#include <grayscale.h>
#include <stdint.h>
#include <stdio.h>

static void print_usage(void) {
  printf("Image Processing - Grayscale and Threshold an image or convert it to "
         "a bitmap\n"
         "Usage: image_process -b|-t path\n"
         "Arguments:\n"
         "\t-b: Convert an image to its bitmap representation and print it.\n"
         "\t-t: Convert and save an image both as its thresholded "
         "representation and as a grayscale\n");
}

static int test_thresholding(char path[static 1]) {
  SDL_Surface *img = SDL_LoadBMP(path);
  if (!img) // error
  {
    printf("Failed to load image: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }

  SDL_Surface *gray = grayscale(img);      // gray
  uint8_t threshold = get_threshold(gray); // thresold of gray with otsu method
  SDL_Surface *bnw = apply_threshold(gray, threshold); // final result

  printf("Threshold = %d\n", threshold);

  SDL_SaveBMP(gray, "gray.bmp");
  SDL_SaveBMP(bnw, "black_n_white.bmp");

  SDL_FreeSurface(bnw);
  SDL_FreeSurface(gray);
  SDL_FreeSurface(img);

  printf("Saved gray.bmp and black_n_white.bmp\n");
  return 1;
}

static int test_p2bmap(const char path[static 1]) {
  int dim = 32 / 8;
  uint8_t a[(32 * 32) / 8];
  if (!path_to_bitmap(path, a, 32, dim)) {
    return 0;
  }
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < dim; ++j) {
      for (int n = 7; n >= 0; --n) {
        printf("%c", (((a[j + (i * dim)]) >> n) & 1) == 0 ? ' ' : '@');
      }
    }
    printf("\n");
  }
  return 1;
}

int main(int argc, char *argv[]) {
  int opt = 0;
  if (argc < 2) {
    printf("Error: requires at least one argument.\n");
    print_usage();
    return 1;
  }

  SDL_Init(SDL_INIT_VIDEO);

  while ((opt = getopt(argc, argv, "b:t:h")) != -1) {
    switch (opt) {
    case 'b':
      test_p2bmap(optarg);
      break;
    case 't':
      test_thresholding(optarg);
      break;
    case ':':
    case '?':
    case 'h':
      print_usage();
      return 0;
    }
  }

  SDL_Quit();
}
