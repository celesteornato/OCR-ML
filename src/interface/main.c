#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>

#define ROTATE_INCREMENT 1.5

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

static bool event_loop(SDL_Renderer *ren, double *angle) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return false;
    }
    if (e.type != SDL_KEYDOWN) {
      return true;
    }

    switch (e.key.keysym.sym) {
    case (SDLK_ESCAPE):
      return false;
    case (SDLK_LEFT):
      *angle -= ROTATE_INCREMENT;
      break;

    case (SDLK_RIGHT):
      *angle += ROTATE_INCREMENT;
      break;

    case (SDLK_r):
      *angle = 0.0;
      break;

    case (SDLK_b):
      save_screenshot(ren, "screenshot.bmp");
      printf("Saved screenshot as screenshot.bmp!\n");
      break;
    default:
      break;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    errx(1, "Usage: %s image.png", argv[0]);
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    errx(1, "SDL_Init: %s", SDL_GetError());
  }

  int flags = IMG_INIT_PNG | IMG_INIT_JPG;
  if ((IMG_Init(flags) & flags) == 0) {
    SDL_Quit();
    errx(1, "IMG_Init: %s", IMG_GetError());
  }

  SDL_Window *win =
      SDL_CreateWindow("Rotation SDL2", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1000, 700, SDL_WINDOW_SHOWN);
  if (!win) {
    SDL_Quit();
    errx(1, "CreateWindow: %s", SDL_GetError());
  }

  SDL_Renderer *ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    SDL_DestroyWindow(win);
    SDL_Quit();
    errx(1, "CreateRenderer: %s", SDL_GetError());
  }

  SDL_Surface *surf = IMG_Load(argv[1]);
  if (!surf) {
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    errx(1, "IMG_Load: %s", IMG_GetError());
  }

  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
  if (!tex) {
    SDL_FreeSurface(surf);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    errx(1, "CreateTextureFromSurface: %s", SDL_GetError());
  }

  int iw = surf->w;
  int ih = surf->h;
  SDL_FreeSurface(surf);

  double angle = 0.0;
  bool running = true;
  while (running) {
    running = event_loop(ren, &angle);

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    int ww;
    int wh;
    SDL_GetRendererOutputSize(ren, &ww, &wh);
    SDL_Rect dst = {(ww - iw) / 2, (wh - ih) / 2, iw, ih};
    SDL_Point center = {iw / 2, ih / 2};
    SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, &center, SDL_FLIP_NONE);
    SDL_RenderPresent(ren);
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
