#include <SDL2/SDL.h>
#include <assert.h>
#include <solver.h>
#include <stdint.h>

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

int main(void) {
  SDL_Init(SDL_INIT_VIDEO);

  assert(test_solver());

  SDL_Quit();
}
