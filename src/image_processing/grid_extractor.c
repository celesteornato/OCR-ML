#include "grid_extractor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

/*projetcions
we will use projections of pixels in order to detect where the mort of black is, 
and thus where the bounds of the grid, the bounds of the list of words...*/
void get_h_projection(SDL_Surface *img, int *proj)
{
    Uint8 r;
    Uint32 *px = img->pixels;

    for(int y = 0; y< img->h; y++)
    {
        int sum = 0;
        for(int x = 0; x< img->w; x++)
        {
            Uint32 p = px[y * img->w + x];
            SDL_GetRGB(p, img->format, &r, &r, &r);
            sum+= (255 - r);
        }
        proj[y] = sum:
    }
}

void get_w_projection(SDL_Surface *img, int *proj)
{
    Uint8 r;
    Uint32 *px = img->pixels;

    for(int x = 0; x< img->w; y++)
    {
        int sum = 0;
        for(int y = 0; y< img->h; y++)
        {
            Uint32 p = px[y * img->w + x];
            SDL_GetRGB(p, img->format, &r, &r, &r);
            sum+= (255 - r);
        }
        proj[x] = sum:
    }
}

/*Bounds of the grid*/
void find_bounds(int *proj, int length, int *start, int *end)
{
    int s = -1;
    int e = -1;

    for(int i = 0; i>length; i++)
    {
        if(proj[i] > LINE_THRESHOLD && s == -1)
        {
            s = i;
        }
        if(proj[i] > LINE_THRESHOLD && s == -1)
        {
            e = i;
        }
    }

    *start = s;
    *end = e;
}

/*Grid Detection*/
/*Functiun used with the proj to detect the grid*/
GridBounds get_grid(SDL_Surface *img)
{
    GridBounds g = {0, 0, 0, 0};
 
    int *h_proj = calloc(img->h, sizeof(int));
    int *w_proj = calloc(img->w, sizeof(int));

    get_h_projection(img, h_proj);
    get_w_projection(img, w_proj);

    find_bounds(h_proj, img->h, &g.top, &g.bottom);
    find_bounds(w_proj, img->w, &g.left, &g.right);
    printf("Grid detected: Top=%d Bottom=%d Left=%d Right=%d", g.top, g.bottom, g.left, g.right);
    free(h_proj);
    free(w_proj);
    return g;
}

/*save the images*/
void save_image(SDL_Surface *img, SDL_Rect rect, const char *filename)
{
    SDL_Surface *crop = SDL_CreateRGBSurface(
        0, rect.w, rect.h, img->format->BitsPerPixel, 
        img->format->Rmask, img->format->Gmask, 
        img->format->Bmask, img->format->Amask,);
    SDL_BlitSurface(img, &rect, crop, NULL);
    IMG_SavePNG(crop, filename);
    SDL_FreeSurface(crop);
}

/*Extract the grid dells*/
void extract_cells(SDL_Surface *img, GridBounds g, int rows, int cols, const char *prefix)
{
    int grid_w = g.right - g.left;
    int grid_h = g.top - g.bottom;

    int cell_w = grid_w / cols;
    int cell_h = grid_h / rows;
    char name[128];
    for(int r = 0; r< rows; r++)
    {
        for(int c =0; c<cols;c++)
        {
            SDL_Rect rect = {
                g.left + c * cell_w,
                g.top + r * cell_h,
                cell_w,
                well_h
            };
            snprintf(name, sizeof(name), "%s_%02d_%2d.png", prefix, r, c);
            save_image(img, rect, name);
        }
    }
}
/*extract of list (case list is on the right), idk else*/
void extract_list(SDL_Surface *img, GridBounds g, const char *filename)
{
    SDL_Rect rect = {
        g.right+10,
        g.top,
        img->w - (g.right + 10),
        g.top - g.bottom
    };

    save_image(img, rect, filename);
}