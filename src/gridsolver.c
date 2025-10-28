#include "neural.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <grayscale.h>
#include <solver.h>
#include <stdint.h>
#include <time.h>


static int test_solver(void) {
  const char grid[][MAX_SIZE] = {
      "DGHEYEUJKQIDIDIDIDID", "FSING1QJJDAAAAAAAAAA", "EYUTTFSODIAAAAHAAAAA",
      "KIORKFLKLAAAAEAAAAAA", "TEFOTOFRTLAALAAAAAAA", "ASZPDPHKLSOLAAAAAAAA",
      "ETYIGIKNXKOAAAAAAAAA", "UKGCFCEDEDAAAAAAAAAA"};

  int rows = 8;
  int cols = 20;
  char word[] = "HELLO";
  char word2[] = "ENTROPIC";
  char word3[] = "ING1";
  const char *list[] = {word, word2, word3};

  // Expected results: {(14,2),(10,6)} , {(3,0)(3,7)},  {(2,1)(5,1)}
  resolve(list, grid, 3, rows, cols);

  printf("\n");
  return 1;
}

static int test_thresholding(char path[static 1]) {
  SDL_Surface *img = SDL_LoadBMP(path);
  if (!img) // error
  {
    printf("Failed to load image: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
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

static int test_neural(void) {
  struct neural_network n = {1};
  srand(time(NULL)); //NOLINT

  train(&n);


  for (size_t i = 0; i < INPUT_SIZE; ++i) {
    printf("Input Node %zu : value of %f\n", i, n.input[i]);
    for (size_t j = 0; j < LAYER1_SIZE; ++j) {
      printf("\tWeight with node %zu : %f\n",j,n.layer1_weights[j][i]);
    }
  }

  printf("%d\n", neural_find_logic(&n, true, false));
  printf("%d\n", neural_find_logic(&n, false, true));
  printf("%d\n", neural_find_logic(&n, true, true));
  printf("%d\n", neural_find_logic(&n, false, false));
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc != 2) // error
  {
    printf("I must take exactly one picture ... :(");
    return 1;
  }
  SDL_Init(SDL_INIT_VIDEO);

  assert(test_solver());
  assert(test_thresholding(argv[1]));
  assert(test_p2bmap(argv[1]));
  assert(test_neural());

  SDL_Quit();
}
