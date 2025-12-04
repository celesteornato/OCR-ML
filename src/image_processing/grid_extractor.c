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

// --- NOUVELLE LOGIQUE DE NETTOYAGE ---

static int clean_lines(int *lines, int count, int min_gap) {
    if (count <= 0) return 0;
    
    int cleaned[count];
    int new_count = 0;
    
    // On ajoute la première ligne
    cleaned[0] = lines[0];
    new_count++;
    
    for (int i = 1; i < count; i++) {
        int diff = lines[i] - lines[i-1];
        
        // Si la ligne est très proche de la précédente (c'est le même trait épais)
        if (diff < min_gap) {
            // On garde la moyenne des deux pour centrer, ou juste la première
            // Ici on ignore simplement la seconde pour ne garder qu'un trait par zone
            continue;
        } else {
            cleaned[new_count++] = lines[i];
        }
    }
    
    // Copie de retour
    for (int i = 0; i < new_count; i++) {
        lines[i] = cleaned[i];
    }
    
    return new_count;
}

// Vérifie si une ligne (ou colonne) est "continue"
// Retourne 1 si c'est une ligne solide (grille), 0 si c'est du texte (avec des trous)
static int is_continuous_line(SDL_Surface *img, int pos, int is_horizontal, int length, int offset_x, int offset_y) {
    int max_gap = 0;
    int current_gap = 0;
    int tolerance = 0; // Nombre de pixels blancs tolérés dans la ligne (bruit)
    
    // On parcourt la ligne
    for (int k = 0; k < length; k++) {
        int x = is_horizontal ? offset_x + k : offset_x + pos;
        int y = is_horizontal ? offset_y + pos : offset_y + k;
        
        if (get_pixel_binary(img, x, y) == 0) { // Noir (Pixel de ligne)
            current_gap = 0;
        } else { // Blanc (Trou ?)
            current_gap++;
            if (current_gap > max_gap) max_gap = current_gap;
        }
    }
    
    // Si le plus grand trou est petit (moins de 5% de la longueur), c'est une ligne continue
    // Une ligne de texte aura des trous énormes entre les lettres.
    if (max_gap < length * 0.05) return 1; 
    return 0;
}


static void extract_cells(SDL_Surface *img, SDL_Rect grid_rect, const char *output_folder)
{
    if (grid_rect.w <= 0 || grid_rect.h <= 0) return;

    // --- 1. DETECTION BASÉE SUR LA CONTINUITÉ ---
    // Au lieu de projeter, on scanne chaque ligne/colonne pour voir si elle est continue
    
    #define MAX_LINES 100
    int h_lines[MAX_LINES];
    int v_lines[MAX_LINES];
    int h_count = 0;
    int v_count = 0;

    // A. Lignes Horizontales
    for (int y = 0; y < grid_rect.h; y++) {
        // Optimisation : On ne vérifie pas chaque pixel si on vient d'en trouver une
        // (Sauf pour déterminer l'épaisseur, mais on simplifie ici)
        
        // On vérifie si la ligne Y est "continue" sur la largeur de la grille
        if (is_continuous_line(img, y, 1, grid_rect.w, grid_rect.x, grid_rect.y)) {
            if (h_count < MAX_LINES) {
                h_lines[h_count++] = y;
                // On saute quelques pixels pour éviter de noter tous les pixels d'un trait épais
                y += 4; 
            }
        }
    }

    // B. Lignes Verticales
    for (int x = 0; x < grid_rect.w; x++) {
        if (x < 5) continue; // Marge de sécurité
        
        if (is_continuous_line(img, x, 0, grid_rect.h, grid_rect.x, grid_rect.y)) {
            if (v_count < MAX_LINES) {
                v_lines[v_count++] = x;
                x += 4; 
            }
        }
    }

    printf("DEBUG: Lignes CONTINUES detectees -> H: %d, V: %d\n", h_count, v_count);

    // --- 2. NETTOYAGE (Juste au cas où) ---
    // Fusionne les lignes détectées à moins de 15px l'une de l'autre
    h_count = clean_lines(h_lines, h_count, 15);
    v_count = clean_lines(v_lines, v_count, 15);
    
    printf("DEBUG: Apres nettoyage -> H: %d, V: %d\n", h_count, v_count);

    // --- 3. DECOUPAGE ---
    char filename[512];
    int cells_saved = 0;
    
    // Paramètres de marge pour rogner le trait noir lui-même
    // Si les lignes sont détectées "au début" du trait noir :
    int line_thickness = 4; // Estimation

    for (int i = 0; i < h_count - 1; i++) {
        for (int j = 0; j < v_count - 1; j++) {
            SDL_Rect cell;
            
            // Le début de la case est après la ligne actuelle
            cell.x = grid_rect.x + v_lines[j] + line_thickness;
            cell.y = grid_rect.y + h_lines[i] + line_thickness;
            
            // La fin de la case est avant la ligne suivante
            int next_x = grid_rect.x + v_lines[j+1];
            int next_y = grid_rect.y + h_lines[i+1];
            
            cell.w = (next_x - cell.x); // On ne retire pas plus, le start de next_x est le début du noir
            cell.h = (next_y - cell.y);

            // Sécurité : ignorer si négatif ou trop petit
            if (cell.w < 10 || cell.h < 10) continue;
            
            // Sécurité : ne pas sortir de l'image
            if (cell.x + cell.w > img->w) cell.w = img->w - cell.x;
            if (cell.y + cell.h > img->h) cell.h = img->h - cell.y;

            sprintf(filename, "%s/cell_%02d_%02d.png", output_folder, i, j);
            save_sub_image(img, cell, filename);
            cells_saved++;
        }
    }
    printf("DEBUG: %d cellules sauvegardees.\n", cells_saved);
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