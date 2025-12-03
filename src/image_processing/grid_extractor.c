#include "../../include/grid_extractor.h"
#include "../../include/grayscale.h" // Inclusion necessaire pour Otsu
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

    return (r > 128) ? 1 : 0;
}

static void save_sub_image(SDL_Surface *src, SDL_Rect rect,
                           const char *filename)
{
    if (rect.w <= 0 || rect.h <= 0)
        return;

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

    // Nouvelle logique basée sur la différence de densité
    int best_split_x = -1;
    double best_score = -1.0;

    // we scan the central zone
    for (int x = width * 0.1; x < width * 0.9; x++)
    {
        // gap detection (white space)
        int is_gap = 1;
        // band of 5 pixels
        for (int k = 0; k < 5 && (x + k) < width; k++)
        {
            int black_pixels = 0;
            for (int y = 0; y < height; y++)
            {
                if (get_pixel_binary(img, x + k, y) == 0)
                    black_pixels++;
            }
            if (black_pixels > height * 0.01) // Tolerance of 1%
            {
                is_gap = 0;
                break;
            }
        }

        if (is_gap)
        {
            // Calcul du contraste de densité (Gauche vs Droite)
            long left_pixels = 0, right_pixels = 0;
            int window = 100; 

            int start_L = (x - window > 0) ? x - window : 0;
            for (int i = start_L; i < x; i++)
                for (int j = 0; j < height; j++)
                    if (get_pixel_binary(img, i, j) == 0)
                        left_pixels++;

            int end_R = (x + window < width) ? x + window : width;
            for (int i = x; i < end_R; i++)
                for (int j = 0; j < height; j++)
                    if (get_pixel_binary(img, i, j) == 0)
                        right_pixels++;

            // Score = Densité Max / Densité Min
            double score = 0;
            if (left_pixels > right_pixels && right_pixels > 0)
                score = (double)left_pixels / right_pixels;
            else if (right_pixels > left_pixels && left_pixels > 0)
                score = (double)right_pixels / left_pixels;

            if (score > best_score)
            {
                best_score = score;
                best_split_x = x;
            }
            x += 5; // Sauter le reste du gap actuel
        }
    }

    if (best_split_x == -1)
    {
        *grid_rect = (SDL_Rect){0, 0, width, height};
        *list_rect = (SDL_Rect){0, 0, 0, 0};
        return;
    }

    // Déterminer le coté grille (le plus dense globalement)
    long left_tot = 0, right_tot = 0;
    for (int i = 0; i < best_split_x; i++)
        for (int j = 0; j < height; j++)
            if (get_pixel_binary(img, i, j) == 0)
                left_tot++;
    for (int i = best_split_x; i < width; i++)
        for (int j = 0; j < height; j++)
            if (get_pixel_binary(img, i, j) == 0)
                right_tot++;

    if (right_tot > left_tot)
    {
        *list_rect = (SDL_Rect){0, 0, best_split_x, height};
        *grid_rect = (SDL_Rect){best_split_x, 0, width - best_split_x, height};
    }
    else
    {
        *grid_rect = (SDL_Rect){0, 0, best_split_x, height};
        *list_rect = (SDL_Rect){best_split_x, 0, width - best_split_x, height};
    }
}

static void extract_cells(SDL_Surface *img, SDL_Rect grid_rect,
                          const char *output_folder)
{
    if (grid_rect.w <= 0 || grid_rect.h <= 0)
        return;

    int *h_proj = calloc((size_t)grid_rect.h, sizeof(int));
    int *v_proj = calloc((size_t)grid_rect.w, sizeof(int));

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

#define MAX_LINES 300 // Augmented for if necessary
    int h_lines[MAX_LINES];
    int v_lines[MAX_LINES];
    int h_count = 0;
    int v_count = 0;

    // Seuil réduit à 5% pour détecter les lignes des images froissées ca te va celeste 0.05 ?
    int threshold_h = (int)(grid_rect.w * 0.05);
    int threshold_v = (int)(grid_rect.h * 0.05);

    // Ajout d'une tolérance pour les lignes discontinues 
    int in_line = 0;
    int gap_tolerance = 0;

    // Lignes Horizontales
    for (int y = 0; y < grid_rect.h; y++)
    {
        if (h_proj[y] > threshold_h)
        {
            if (!in_line)
            {
                if (h_count < MAX_LINES)
                    h_lines[h_count++] = y;
                in_line = 1;
            }
            gap_tolerance = 0;
        }
        else
        {
            if (in_line)
            {
                gap_tolerance++;
                if (gap_tolerance > 2) // Si trou > 2 pixels, fin de ligne
                    in_line = 0;
            }
        }
    }

    // Lignes Verticales
    in_line = 0;
    gap_tolerance = 0;
    for (int x = 0; x < grid_rect.w; x++)
    {
        // Ignorer le bord gauche immédiat
        if (!in_line && x < 5 && v_proj[x] > threshold_v)
            continue;

        if (v_proj[x] > threshold_v)
        {
            if (!in_line)
            {
                if (v_count < MAX_LINES)
                    v_lines[v_count++] = x;
                in_line = 1;
            }
            gap_tolerance = 0;
        }
        else
        {
            if (in_line)
            {
                gap_tolerance++;
                if (gap_tolerance > 2)
                    in_line = 0;
            }
        }
    }

    // Calcul taille moyenne pour filtrer le bruit (double colonnes)
    int min_cell_w = (v_count > 1) ? (v_lines[v_count - 1] - v_lines[0]) / v_count / 2 : 10;
    int min_cell_h = (h_count > 1) ? (h_lines[h_count - 1] - h_lines[0]) / h_count / 2 : 10;
    if (min_cell_w < 5) min_cell_w = 10;
    if (min_cell_h < 5) min_cell_h = 10;

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

            //Filtre plus strict sur la taille minimale
            if (cell.w < min_cell_w || cell.h < min_cell_h)
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

    // Intégration complète Niveaux de gris + Otsu
    SDL_Surface *gray_img = grayscale(img);
    SDL_FreeSurface(img); 

    uint8_t threshold = get_threshold(gray_img);
    SDL_Surface *bin_img = apply_threshold(gray_img, threshold);
    SDL_FreeSurface(gray_img);
    
    // Conversion finale en RGBA32 pour le traitement
    SDL_Surface *fmt_img = SDL_ConvertSurfaceFormat(bin_img, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(bin_img);

    SDL_Rect grid_rect;
    SDL_Rect list_rect;

    printf("[GridExtractor] Analyzing image structure...\n");
    split_grid_and_list(fmt_img, &grid_rect, &list_rect);

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