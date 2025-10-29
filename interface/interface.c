#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdio.h>

static int save_screenshot(SDL_Renderer *ren, const char *filename) {
  int w;
  int h;
  if (SDL_GetRendererOutputSize(ren, &w, &h) != 0) {
    return -1;
  }

  SDL_Surface *shot =
      SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
  if (!shot) {
    return -2;
  }

  if (SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, shot->pixels,
                           shot->pitch) != 0) {
    SDL_FreeSurface(shot);
    return -3;
  }

  int rc = SDL_SaveBMP(shot, filename);
  SDL_FreeSurface(shot);
  return rc;
}

static int event_loop(SDL_Renderer *ren, double *angle) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return 0;
    }
    if (e.type != SDL_KEYDOWN) {
      return 1;
    }

    switch (e.key.keysym.sym) {
    case (SDLK_ESCAPE):
      return 0;
    case (SDLK_LEFT):
      *angle -= 5.0;
      break;

    case (SDLK_RIGHT):
      *angle += 5.0;
      break;

    case (SDLK_r):
      *angle = 0.0;
      break;

    case (SDLK_b):
      save_screenshot(ren, "screenshot.bmp");
      break;
    default:
      break;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Utilisation: %s image.png\n", argv[0]);
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init erreur: %s\n", SDL_GetError());
    return 1;
  }

  int flags = IMG_INIT_PNG | IMG_INIT_JPG;
  if ((IMG_Init(flags) & flags) == 0) {
    fprintf(stderr, "IMG_Init erreur: %s\n", IMG_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Window *win =
      SDL_CreateWindow("Rotation SDL2", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1000, 700, SDL_WINDOW_SHOWN);
  if (!win) {
    fprintf(stderr, "CreateWindow: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    fprintf(stderr, "CreateRenderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_Surface *surf = IMG_Load(argv[1]);
  if (!surf) {
    fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
  if (!tex) {
    fprintf(stderr, "CreateTextureFromSurface: %s\n", SDL_GetError());
    SDL_FreeSurface(surf);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  int iw = surf->w;
  int ih = surf->h;
  SDL_FreeSurface(surf);

  double angle = 0.0;
  while (event_loop(ren, &angle)) {
  }

  if (tex) {
    SDL_DestroyTexture(tex);
  }
  if (ren) {
    SDL_DestroyRenderer(ren);
  }
  if (win) {
    SDL_DestroyWindow(win);
  }

  IMG_Quit();
  SDL_Quit();
  return 0;
}
