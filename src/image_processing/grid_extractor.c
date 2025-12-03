#include "../../include/grid_extractor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

// --- INTERNAL FUNCTIONS ---

static int get_pixel_binary(SDL_Surface *surface, int x, int y)
{
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return 1;

    Uint32 *pixels = (Uint32 *)surface->pixels;
    Uint32 pixel = pixels[(y * surface->w) + x];
    Uint8 r, g, b;
    SDL_GetRGB(pixel, surface->format, &r, &g, &b);

    // Threshold: >128 is white, <128 is black
    return (r > 128) ? 1 : 0;
}

// Saves a specific rectangle of the surface to a PNG file
static void save_sub_image(SDL_Surface *src, SDL_Rect rect,
                           const char *filename)
{
    if (rect.w <= 0 || rect.h <= 0)
    {
        return;
    }

    SDL_Surface *sub = SDL_CreateRGBSurface(0, rect.w, rect.h, 32, 0x00FF0000,
                                            0x0000FF00, 0x000000FF, 0xFF000000);

    SDL_Rect src_rect = rect;
    SDL_Rect dest_rect = {0, 0, rect.w, rect.h};

    SDL_BlitSurface(src, &src_rect, sub, &dest_rect);
    IMG_SavePNG(sub, filename);
    SDL_FreeSurface(sub);
}

// --- CORE ---

static void split_grid_and_list(SDL_Surface *img, SDL_Rect *grid_rect,
                                SDL_Rect *list_rect)
{
    int width = img->w;
    int height = img->h;

    int max_gap = 0;
    int current_gap = 0;
    int split_x = -1;

    // Vertical Projection: Scan columns to find the big vertical white gap
    for (int x = 0; x < width; x++)
    {
        int black_pixels = 0;
        for (int y = 0; y < height; y++)
        {
            if (get_pixel_binary(img, x, y) == 0)
            {
                black_pixels++;
            }
        }
        // If column has very little ink, count it as gap
        if (black_pixels < 2)
        {
            current_gap++;
        }
        else
        {
            // Check if this was the largest gap found so far
            if (current_gap > max_gap && x > width * 0.1 && x < width * 0.9)
            {
                max_gap = current_gap;
                split_x = x - (current_gap / 2);
            }
            current_gap = 0;
        }
    }
    // If no split found, assume everything is grid
    if (split_x == -1)
    {
        *grid_rect = (SDL_Rect){0, 0, width, height};
        *list_rect = (SDL_Rect){0, 0, 0, 0};
        return;
    }

    // Determine which side is the grid (the denser side)
    long left_density = 0;
    long right_density = 0;
    for (int x = 0; x < split_x; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (get_pixel_binary(img, x, y) == 0)
            {
                left_density++;
            }
        }
    }

    for (int x = split_x; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (get_pixel_binary(img, x, y) == 0)
            {
                right_density++;
            }
        }
    }

    if (right_density > left_density)
    {
        // Grid is on the Right
        *list_rect = (SDL_Rect){0, 0, split_x, height};
        *grid_rect = (SDL_Rect){split_x, 0, width - split_x, height};
    }
    else
    {
        // Grid is on the Left
        *grid_rect = (SDL_Rect){0, 0, split_x, height};
        *list_rect = (SDL_Rect){split_x, 0, width - split_x, height};
    }
}

static void extract_cells(SDL_Surface *img, SDL_Rect grid_rect,
                          const char *output_folder)
{
    if (grid_rect.w <= 0 || grid_rect.h <= 0)
    {
        return;
    }

    // Allocate histogram arrays
    int *h_proj = calloc((size_t)grid_rect.h, sizeof(int));
    int *v_proj = calloc((size_t)grid_rect.w, sizeof(int));

    // Fill Histograms
    for (int y = 0; y < grid_rect.h; y++)
    {
        for (int x = 0; x < grid_rect.w; x++)
        {
            if (get_pixel_binary(img, grid_rect.x + x, grid_rect.y + y) == 0)
            {
                h_proj[y]++;
                v_proj[x]++;
            }
        }
    }

// Detect Lines (Peaks in histograms)
#define MAX_LINES 200
    int h_lines[MAX_LINES];
    int v_lines[MAX_LINES];
    int h_count = 0;
    int v_count = 0;
    // A grid line must span at least 40% of the dimension
    int threshold_h = (2 * grid_rect.w) / 5;
    int threshold_v = (2 * grid_rect.h) / 5;

    int in_line = 0;
    // Find Horizontal Lines
    for (int y = 0; y < grid_rect.h; y++)
    {
        if (h_proj[y] > threshold_h)
        {
            if (!in_line)
            {
                if (h_count < MAX_LINES)
                {
                    h_lines[h_count++] = y;
                }
                in_line = 1;
            }
        }
        else
        {
            in_line = 0;
        }
    }

    // Find Vertical Lines
    in_line = 0;
    for (int x = 0; x < grid_rect.w; x++)
    {
	if(x < 5 && v_proj[x] > threshold_v)
	{
	  continue;
	}
        if (v_proj[x] > threshold_v)
        {
            if (!in_line)
            {
                if (v_count < MAX_LINES)
                {
                    v_lines[v_count++] = x;
                }
                in_line = 1;
            }
        }
        else
        {
            in_line = 0;
        }
    }

    // Slice Cells based on intersection of lines
    char filename[512];

    for (int i = 0; i < h_count - 1; i++)
    {
        for (int j = 0; j < v_count - 1; j++)
        {
            SDL_Rect cell;
            cell.x = grid_rect.x + v_lines[j];
            cell.y = grid_rect.y + h_lines[i];
            cell.w = v_lines[j + 1] - v_lines[j];
            cell.h = h_lines[i + 1] - h_lines[i];

            // Ignore noise/tiny boxes
            if (cell.w < 5 || cell.h < 5)
            {
                continue;
            }

            sprintf(filename, "%s/cell_%02d_%02d.png", output_folder, i, j);
            save_sub_image(img, cell, filename);
        }
    }

    free(h_proj);
    free(v_proj);
}

// --- MAIN FUNCTION ---

void extract_grid_data(const char *input_path, const char *output_folder)
{
    SDL_Surface *img = IMG_Load(input_path);
    if (!img)
    {
        printf("Error loading image (%s): %s\n", input_path, IMG_GetError());
        return;
    }

    // Convert to RGBA32 for consistent pixel access
    SDL_Surface *fmt_img =
        SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(img);

    SDL_Rect grid_rect;
    SDL_Rect list_rect;

    printf("[GridExtractor] Analyzing image structure...\n");
    split_grid_and_list(fmt_img, &grid_rect, &list_rect);

    // Save crops
    char path[512];
    sprintf(path, "%s/grid_crop.png", output_folder);
    save_sub_image(fmt_img, grid_rect, path);
    printf("[GridExtractor] Grid crop saved.\n");

    sprintf(path, "%s/list_crop.png", output_folder);
    save_sub_image(fmt_img, list_rect, path);
    printf("[GridExtractor] List crop saved.\n");

    printf("[GridExtractor] Extracting cells...\n");
    extract_cells(fmt_img, grid_rect, output_folder);
    printf("[GridExtractor] Cells extracted to %s.\n", output_folder);

    SDL_FreeSurface(fmt_img);
}
