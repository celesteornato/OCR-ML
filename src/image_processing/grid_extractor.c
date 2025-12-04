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

// Cette fonction nettoie les tableaux de lignes pour fusionner les doublons
// (Ex: bord gauche et bord droit d'un trait épais détectés comme 2 lignes)
static int clean_lines(int *lines, int count, int min_gap) {
    if (count <= 0) return 0;
    
    int cleaned[count];
    int new_count = 0;
    
    cleaned[0] = lines[0];
    new_count++;
    
    for (int i = 1; i < count; i++) {
        // Si la ligne actuelle est trop proche de la précédente, on l'ignore (fusion)
        if (lines[i] - lines[i-1] > min_gap) {
            cleaned[new_count++] = lines[i];
        }
    }
    
    // On remet les lignes propres dans le tableau original
    for (int i = 0; i < new_count; i++) {
        lines[i] = cleaned[i];
    }
    
    return new_count;
}

// --- FONCTION EXTRACT CELLS MODIFIÉE ---

static void extract_cells(SDL_Surface *img, SDL_Rect grid_rect,
                          const char *output_folder)
{
    if (grid_rect.w <= 0 || grid_rect.h <= 0) return;

    int *h_proj = calloc((size_t)grid_rect.h, sizeof(int));
    int *v_proj = calloc((size_t)grid_rect.w, sizeof(int));

    // Remplissage Histogrammes
    for (int y = 0; y < grid_rect.h; y++) {
        for (int x = 0; x < grid_rect.w; x++) {
            if (get_pixel_binary(img, grid_rect.x + x, grid_rect.y + y) == 0) {
                h_proj[y]++;
                v_proj[x]++;
            }
        }
    }

    #define MAX_LINES 500
    int h_lines[MAX_LINES];
    int v_lines[MAX_LINES];
    int h_count = 0;
    int v_count = 0;

    // Seuil très bas pour capter même les lignes faibles
    int threshold_h = (int)(grid_rect.w * 0.05);
    int threshold_v = (int)(grid_rect.h * 0.05);

    // --- DETECTION BRUTE ---
    
    int in_line = 0;
    // Horizontales
    for (int y = 0; y < grid_rect.h; y++) {
        if (h_proj[y] > threshold_h) {
            if (!in_line && h_count < MAX_LINES) {
                h_lines[h_count++] = y;
                in_line = 1;
            }
        } else { in_line = 0; }
    }

    in_line = 0;
    // Verticales
    for (int x = 0; x < grid_rect.w; x++) {
        // Ignorer la marge gauche extrême (bruit de découpe)
        if (x < 5) continue; 

        if (v_proj[x] > threshold_v) {
            if (!in_line && v_count < MAX_LINES) {
                v_lines[v_count++] = x;
                in_line = 1;
            }
        } else { in_line = 0; }
    }

    // --- DEBUG INFO ---
    printf("DEBUG: Lignes detectees AVANT nettoyage -> H: %d, V: %d\n", h_count, v_count);

    // --- NETTOYAGE (FUSION) ---
    // Si deux lignes sont séparées de moins de 10 pixels, c'est la meme ligne épaisse
    h_count = clean_lines(h_lines, h_count, 10);
    v_count = clean_lines(v_lines, v_count, 10);

    printf("DEBUG: Lignes detectees APRES nettoyage -> H: %d, V: %d\n", h_count, v_count);

    // --- DECOUPAGE ---
    
    // Calcul taille moyenne (juste pour info debug)
    int avg_w = (v_count > 1) ? (v_lines[v_count-1] - v_lines[0]) / (v_count-1) : 0;
    int avg_h = (h_count > 1) ? (h_lines[h_count-1] - h_lines[0]) / (h_count-1) : 0;
    
    // Filtre de sécurité minimaliste (5x5 pixels)
    // On évite le filtre "moyenne" pour l'instant car il bloque l'image 2 si la grille est mal détectée
    int min_w = 5; 
    int min_h = 5;

    char filename[512];
    int cells_saved = 0;

    for (int i = 0; i < h_count - 1; i++) {
        for (int j = 0; j < v_count - 1; j++) {
            SDL_Rect cell;
            
            // On prend l'espace ENTRE la ligne J et la ligne J+1
            // On ajoute un petit offset (+2) pour ne pas prendre le noir de la ligne elle-même
            cell.x = grid_rect.x + v_lines[j] + 2; 
            cell.y = grid_rect.y + h_lines[i] + 2;
            
            // La largeur est la distance entre les lignes, moins l'épaisseur supposée (4px)
            cell.w = (v_lines[j + 1] - v_lines[j]) - 4;
            cell.h = (h_lines[i + 1] - h_lines[i]) - 4;

            if (cell.w < min_w || cell.h < min_h) continue;

            sprintf(filename, "%s/cell_%02d_%02d.png", output_folder, i, j);
            save_sub_image(img, cell, filename);
            cells_saved++;
        }
    }
    
    printf("DEBUG: Nombre total de cellules sauvegardees : %d\n", cells_saved);

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