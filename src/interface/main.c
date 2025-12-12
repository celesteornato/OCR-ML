#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>

#define rotate_increment 1.5

static int save_screenshot(sdl_renderer *ren, const char *filename) {
  int w;
  int h;
  if (sdl_getrendereroutputsize(ren, &w, &h) != 0) {
    return -1;
  }
  sdl_surface *shot =
      sdl_creatergbsurfacewithformat(0, w, h, 32, sdl_pixelformat_argb8888);
  if (!shot) {
    return -2;
  }
  if (sdl_renderreadpixels(ren, null, sdl_pixelformat_argb8888, shot->pixels,
                           shot->pitch) != 0) {
    sdl_freesurface(shot);
    return -3;
  }
  int rc = sdl_savebmp(shot, filename);
  sdl_freesurface(shot);
  return rc;
}

static bool load_texture(sdl_renderer *ren, const char *path,
                         sdl_texture **tex, int *iw, int *ih) {
  sdl_surface *surf = img_load(path);
  if (!surf) {
    fprintf(stderr, "img_load: %s\n", img_geterror());
    return false;
  }
  sdl_texture *new_tex = sdl_createtexturefromsurface(ren, surf);
  if (!new_tex) {
    sdl_freesurface(surf);
    fprintf(stderr, "createtexturefromsurface: %s\n", sdl_geterror());
    return false;
  }
  if (*tex) {
    sdl_destroytexture(*tex);
  }

  *tex = new_tex;
  *iw = surf->w;
  *ih = surf->h;

  sdl_freesurface(surf);
  return true;
}

static bool event_loop(sdl_renderer *ren, double *angle,
                       sdl_texture **tex, int *iw, int *ih) {
  sdl_event e;
  while (sdl_pollevent(&e)) {
    if (e.type == sdl_quit) {
      return false;
    }

    if (e.type == sdl_keydown) {
      switch (e.key.keysym.sym) {
      case (sdlk_escape):
        return false;
      case (sdlk_left):
        *angle -= rotate_increment;
        break;

      case (sdlk_right):
        *angle += rotate_increment;
        break;

      case (sdlk_r):
        *angle = 0.0;
        break;

      case (sdlk_b):
        save_screenshot(ren, "screenshot.bmp");
        printf("saved screenshot as screenshot.bmp!\n");
        break;
      default:
        break;
      }
      return true;
    }

    if (e.type == sdl_dropfile) {
      char *path = e.drop.file;
      load_texture(ren, path, tex, iw, ih);
      sdl_free(path);
      return true;
    }

    return true;
  }
  return 1;
}

int main(int argc, char **argv) {
  if (sdl_init(sdl_init_video) != 0) {
    errx(1, "sdl_init: %s", sdl_geterror());
  }

  int flags = img_init_png | img_init_jpg;
  if ((img_init(flags) & flags) == 0) {
    sdl_quit();
    errx(1, "img_init: %s", img_geterror());
  }

  sdl_window *win =
      sdl_createwindow("rotation sdl2", sdl_windowpos_centered,
                       sdl_windowpos_centered, 1000, 700, sdl_window_shown);
  if (!win) {
    sdl_quit();
    errx(1, "createwindow: %s", sdl_geterror());
  }

  sdl_renderer *ren = sdl_createrenderer(
      win, -1, sdl_renderer_accelerated | sdl_renderer_presentvsync);
  if (!ren) {
    sdl_destroywindow(win);
    sdl_quit();
    errx(1, "createrenderer: %s", sdl_geterror());
  }
  sdl_texture *tex = null;
  int iw = 0;
  int ih = 0;
  if (argc >= 2) {
    if (!load_texture(ren, argv[1], &tex, &iw, &ih)) {
      printf("could not load %s, drag & drop an image into the window.\n",
             argv[1]);
    }
  } else {
    printf("drag & drop an image into the window.\n");
  }
  double angle = 0.0;
  bool running = true;
  while (running) {
    running = event_loop(ren, &angle, &tex, &iw, &ih);
    sdl_setrenderdrawcolor(ren, 255, 255, 255, 255);
    sdl_renderclear(ren);
    if (tex) {
      int ww;
      int wh;
      sdl_getrendereroutputsize(ren, &ww, &wh);
      double sx = (double)ww / (double)iw;
      double sy = (double)wh / (double)ih;
      double s = (sx < sy) ? sx : sy;
      int dw = (int)(iw * s);
      int dh = (int)(ih * s);
      sdl_rect dst = {(ww - dw) / 2, (wh - dh) / 2, dw, dh};
      sdl_point center = {dw / 2, dh / 2};
      sdl_rendercopyex(ren, tex, null, &dst, angle, &center, sdl_flip_none);
    }
    sdl_renderpresent(ren);
  }
  if (tex) {
    sdl_destroytexture(tex);
  }
  if (ren) {
    sdl_destroyrenderer(ren);
  }
  if (win) {
    sdl_destroywindow(win);
  img_quit();
  sdl_quit();
  return 0;
}
