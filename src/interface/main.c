#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>

#define ROTATE_INCREMENT 1.5

#define UI_W 220
#define BTN_W 160
#define BTN_H 60

static bool point_in_rect(int x, int y, SDL_Rect *r)
{
    return x >= r->x && x <= r->x + r->w && y >= r->y && y <= r->y + r->h;
}

static int save_screenshot(SDL_Renderer *ren, const char *filename)
{
    int w;
    int h;
    if (SDL_GetRendererOutputSize(ren, &w, &h) != 0)
    {
        return -1;
    }

    SDL_Surface *shot =
        SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!shot)
    {
        return -2;
    }

    if (SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, shot->pixels,
                             shot->pitch) != 0)
    {
        SDL_FreeSurface(shot);
        return -3;
    }

    int rc = SDL_SaveBMP(shot, filename);
    SDL_FreeSurface(shot);
    return rc;
}

static void load_image(SDL_Renderer *ren, const char *path, SDL_Texture **tex,
                       int *iw, int *ih)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf)
    {
        warnx("IMG_Load(%s): %s", path, IMG_GetError());
        return;
    }

    SDL_Texture *new_tex = SDL_CreateTextureFromSurface(ren, surf);
    if (!new_tex)
    {
        warnx("SDL_CreateTextureFromSurface: %s", SDL_GetError());
        SDL_FreeSurface(surf);
        return;
    }

    if (*tex)
    {
        SDL_DestroyTexture(*tex);
    }

    *tex = new_tex;
    *iw = surf->w;
    *ih = surf->h;

    SDL_FreeSurface(surf);
    printf("loaded %s (%dx%d)\n", path, *iw, *ih);
}

static void render_text_center(SDL_Renderer *ren, TTF_Font *font,
                               const char *text, SDL_Rect *where)
{
    SDL_Color col = {255, 255, 255, 255};

    SDL_Surface *s = TTF_RenderUTF8_Blended(font, text, col);
    if (!s)
    {
        warnx("TTF_RenderUTF8_Blended: %s", TTF_GetError());
        return;
    }

    SDL_Texture *t = SDL_CreateTextureFromSurface(ren, s);
    if (!t)
    {
        warnx("SDL_CreateTextureFromSurface(text): %s", SDL_GetError());
        SDL_FreeSurface(s);
        return;
    }

    SDL_Rect dst = {where->x + ((where->w - s->w) / 2),
                    where->y + ((where->h - s->h) / 2), s->w, s->h};

    SDL_FreeSurface(s);

    SDL_RenderCopy(ren, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

static bool event_loop(SDL_Renderer *ren, double *angle, SDL_Texture **tex,
                       int *iw, int *ih, SDL_Rect *solve_btn)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            return false;
        }

        if (e.type == SDL_DROPFILE)
        {
            char *file = e.drop.file;
            load_image(ren, file, tex, iw, ih);
            SDL_free(file);
            continue;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int mx = e.button.x;
            int my = e.button.y;

            if (point_in_rect(mx, my, solve_btn))
            {
                printf("LE BOUTTTTONNNNNNN\n");
                /* marque le code ici */
            }
        }

        if (e.type != SDL_KEYDOWN)
        {
            continue;
        }

        switch (e.key.keysym.sym)
        {
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

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        errx(1, "SDL_Init: %s", SDL_GetError());
    }
    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(flags) & flags) == 0)
    {
        SDL_Quit();
        errx(1, "IMG_Init: %s", IMG_GetError());
    }
    if (TTF_Init() != 0)
    {
        IMG_Quit();
        SDL_Quit();
        errx(1, "TTF_Init: %s", TTF_GetError());
    }
    SDL_Window *win =
        SDL_CreateWindow("rotation sdl2", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_SHOWN);
    if (!win)
    {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        errx(1, "SDL_CreateWindow: %s", SDL_GetError());
    }

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        SDL_DestroyWindow(win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        errx(1, "SDL_CreateRenderer: %s", SDL_GetError());
    }

    TTF_Font *font = TTF_OpenFont("assets/font.ttf", 28);
    if (!font)
    {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        errx(1, "TTF_OpenFont: %s", SDL_GetError());
    }
    SDL_Texture *tex = NULL;
    int iw = 0;
    int ih = 0;

    if (argc >= 2)
    {
        load_image(ren, argv[1], &tex, &iw, &ih);
    }
    else
    {
        printf("no image argument. drag & drop an image onto the window.\n");
    }

    SDL_Rect solve_btn = {0, 0, BTN_W, BTN_H};
    double angle = 0.0;
    bool running = true;
    while (running)
    {
        running = event_loop(ren, &angle, &tex, &iw, &ih, &solve_btn);
        int ww;
        int wh;
        SDL_GetRendererOutputSize(ren, &ww, &wh);
        SDL_Rect image_area = {0, 0, ww - UI_W, wh};
        SDL_Rect ui_area = {ww - UI_W, 0, UI_W, wh};
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);
        if (tex && iw > 0 && ih > 0)
        {
            double sx = (double)image_area.w / (double)iw;
            double sy = (double)image_area.h / (double)ih;
            double s = (sx < sy) ? sx : sy;

            int dw = (int)(iw * s);
            int dh = (int)(ih * s);

            SDL_Rect dst = {image_area.x + ((image_area.w - dw) / 2),
                            image_area.y + ((image_area.h - dh) / 2), dw, dh};
            SDL_Point center = {dw / 2, dh / 2};
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, &center,
                             SDL_FLIP_NONE);
        }
        SDL_SetRenderDrawColor(ren, 235, 235, 235, 255);
        SDL_RenderFillRect(ren, &ui_area);
        SDL_SetRenderDrawColor(ren, 180, 180, 180, 255);
        SDL_RenderDrawRect(ren, &ui_area);
        solve_btn.x = ui_area.x + (ui_area.w - BTN_W) / 2;
        solve_btn.y = 60;
        solve_btn.w = BTN_W;
        solve_btn.h = BTN_H;
        SDL_SetRenderDrawColor(ren, 40, 120, 220, 255);
        SDL_RenderFillRect(ren, &solve_btn);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderDrawRect(ren, &solve_btn);
        render_text_center(ren, font, "SOLVE", &solve_btn);
        SDL_RenderPresent(ren);
    }
    if (tex)
    {
        SDL_DestroyTexture(tex);
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
