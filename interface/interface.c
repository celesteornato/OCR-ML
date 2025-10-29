#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

static int save_screenshot(SDL_Renderer *ren, const char *filename)
{
    int w, h;
    if (SDL_GetRendererOutputSize(ren, &w, &h) != 0)
    {
        return -1;
    }

    SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!shot)
    {
        return -2;
    }

    if (SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888, shot->pixels, shot->pitch) != 0)
    {
        SDL_FreeSurface(shot);
        return -3;
    }

    int rc = SDL_SaveBMP(shot, filename);
    SDL_FreeSurface(shot);
    return rc;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Utilisation: %s image.png\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init erreur: %s\n", SDL_GetError());
        return 1;
    }

    int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(flags) & flags) == 0)
    {
        fprintf(stderr, "IMG_Init erreur: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Rotation SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 700, SDL_WINDOW_SHOWN);
    if (!win)
    {
        fprintf(stderr, "CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        fprintf(stderr, "CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surf = IMG_Load(argv[1]);
    if (!surf)
    {
        fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (!tex)
    {
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
    int running = 1;
    int take_screenshot = 0;

    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = 0;
                }
                if (e.key.keysym.sym == SDLK_LEFT)
                {
                    angle -= 5.0;
                }
                if (e.key.keysym.sym == SDLK_RIGHT)
                {
                    angle += 5.0;
                }
                if (e.key.keysym.sym == SDLK_r)
                {
                    angle = 0.0;
                }
                if (e.key.keysym.sym == SDLK_b)
                {
                    take_screenshot = 1;
                }
            }
        }

        int ww
        int wh;
        SDL_GetRendererOutputSize(ren, &ww, &wh);
        SDL_Rect dst = { (ww - iw)/2, (wh - ih)/2, iw, ih };

        SDL_SetRenderDrawColor(ren, 20, 20, 20, 255);
        SDL_RenderClear(ren);
        SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);

        if (take_screenshot)
        {
            save_screenshot(ren, "export.bmp");
            take_screenshot = 0;
        }

        SDL_RenderPresent(ren);
    }

    if (tex)
    {
        SDL_DestroyTexture(tex);
    }
    if (ren)
    {
        SDL_DestroyRenderer(ren);
    }
    if (win)
    {
        SDL_DestroyWindow(win);
    }

    IMG_Quit();
    SDL_Quit();
    return 0;
}