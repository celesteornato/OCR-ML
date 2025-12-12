#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

  if (SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888,
                           shot->pixels, shot->pitch) != 0) {
    SDL_FreeSurface(shot);
    return -3;
  }

  int rc = SDL_SaveBMP(shot, filename);
  SDL_FreeSurface(shot);
  return rc;
}

static void load_image(SDL_Renderer *ren, const char *path,
                       SDL_Texture **tex, int *iw, int *ih) {
  SDL_Surface *surf = IMG_Load(path);
  if (!surf) {
    warnx("IMG_Load(%s): %s", path, IMG_GetError());
    return;
  }

  SDL_Texture *new_tex = SDL_CreateTextureFromSurface(ren, surf);
  if (!new_tex) {
    warnx("SDL_CreateTextureFromSurface: %s", SDL_GetError());
    SDL_FreeSurface(surf);
    return;
  }

  if (*tex) {
    SDL_DestroyTexture(*tex);
  }
  *tex = new_tex;
  *iw = surf->w;
  *ih = surf->h;
  SDL_FreeSurface(surf);
  printf("loaded %s (%dx%d)\n", path, *iw, *ih);
}

static bool event_loop(SDL_Renderer *ren, double *angle,
                       SDL_Texture **tex, int *iw, int *ih) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return false;
    }
    if (e.type == SDL_DROPFILE) {
      char *file = e.drop.file;
      load_image(ren, file, tex, iw, ih);
      SDL_free(file);
      continue;
    }

    if (e.type != SDL_KEYDOWN) {
      continue;
    }

    switch (e.key.keysym.sym) {
    case SDLK_ESCAPE:
      return false;
    case SDLK_LEFT:
      *angle -= ROTATE_INCREMENT;
      break;
    case SDLK_RIGHT:
      *angle += ROTATE_INCREMENT;
      break;
    case SDLK_r:
      *angle = 0.0;
      break;
    case SDLK_b:
      save_screenshot(ren, "screenshot.bmp");
      printf("saved screenshot as screenshot.bmp!\n");
      break;
    default:
      break;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    errx(1, "SDL_Init: %s", SDL_GetError());
  }
  int flags = IMG_INIT_PNG | IMG_INIT_JPG;
  if ((IMG_Init(flags) & flags) == 0) {
    SDL_Quit();
    errx(1, "IMG_Init: %s", IMG_GetError());
  }
  SDL_Window *win =
      SDL_CreateWindow("rotation sdl2",
                       SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED,
                       1000, 700,
                       SDL_WINDOW_SHOWN);
  if (!win) {
    SDL_Quit();
    errx(1, "SDL_CreateWindow: %s", SDL_GetError());
  }
  SDL_Renderer *ren = SDL_CreateRenderer(
      win, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    SDL_DestroyWindow(win);
    SDL_Quit();
    errx(1, "SDL_CreateRenderer: %s", SDL_GetError());
  }
  SDL_Texture *tex = NULL;
  int iw = 0;
  int ih = 0;
  if (argc >= 2) {
    load_image(ren, argv[1], &tex, &iw, &ih);
  } else {
    printf("no image argument. drag & drop an image onto the window.\n");
  }
  double angle = 0.0;
  bool running = true;
  while (running) {
    running = event_loop(ren, &angle, &tex, &iw, &ih);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderClear(ren);
    if (tex && iw > 0 && ih > 0) {
      int ww;
      int wh;
      SDL_GetRendererOutputSize(ren, &ww, &wh);
      double sx = (double)ww / (double)iw;
      double sy = (double)wh / (double)ih;
      double s = (sx < sy) ? sx : sy;
      int dw = (int)(iw * s);
      int dh = (int)(ih * s);

      SDL_Rect dst = {(ww - dw) / 2, (wh - dh) / 2, dw, dh};
      SDL_Point center = {dw / 2, dh / 2};

      SDL_RenderCopyEx(ren, tex, NULL, &dst,
                       angle, &center, SDL_FLIP_NONE);
    }
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

