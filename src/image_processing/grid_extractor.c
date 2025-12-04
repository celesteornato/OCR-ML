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

// --- FONCTION DE VALIDATION (Anti-Texte) ---

// Vérifie si une ligne contient un segment noir assez long pour être une grille.
// Les lignes de texte sont composées de petits segments (lettres) séparés par du blanc.
static int is_line_solid(SDL_Surface *img, int pos, int is_horizontal, SDL_Rect area) {
    int max_run = 0;     // Longueur max du segment noir continu trouvé
    int current_run = 0; // Segment actuel
   
    // Définir la longueur à scanner
    int limit = is_horizontal ? area.w : area.h;
   
    // Pour éviter de scanner toute la largeur (lent), on peut scanner le centre
    // ou scanner toute la ligne. Ici on scanne tout pour la précision.
    for (int k = 0; k < limit; k++) {
        int x = is_horizontal ? area.x + k : area.x + pos;
        int y = is_horizontal ? area.y + pos : area.y + k;
       
        // Si pixel Noir (attention, get_pixel_binary renvoie 0 pour noir)
        if (get_pixel_binary(img, x, y) == 0) {
            current_run++;
        } else {
            // Pixel Blanc : fin du segment
            if (current_run > max_run) max_run = current_run;
            current_run = 0;
        }
    }
    // Check final (si la ligne finit par du noir)
    if (current_run > max_run) max_run = current_run;

    // HEURISTIQUE MAGIQUE :
    // Une ligne de grille doit avoir un segment continu d'au moins 1/8eme de la taille totale.
    // Une lettre (même un "W" large) dépasse rarement 1/15eme ou 1/20eme de la grille.
    // Pour l'image 1 (lignes fines mais continues), ça passera.
    // Pour le texte, ça échouera.
    return (max_run > limit / 8);
}

// --- FONCTION PRINCIPALE ---

static void extract_cells(SDL_Surface *img, SDL_Rect grid_rect, const char *output_folder)
{
    if (grid_rect.w <= 0 || grid_rect.h <= 0) return;

    // 1. Histogrammes (Méthode classique)
    int *h_proj = calloc((size_t)grid_rect.h, sizeof(int));
    int *v_proj = calloc((size_t)grid_rect.w, sizeof(int));

    for (int y = 0; y < grid_rect.h; y++) {
        for (int x = 0; x < grid_rect.w; x++) {
            if (get_pixel_binary(img, grid_rect.x + x, grid_rect.y + y) == 0) {
                h_proj[y]++;
                v_proj[x]++;
            }
        }
    }

    #define MAX_LINES 100
    int h_lines[MAX_LINES];
    int v_lines[MAX_LINES];
    int h_count = 0;
    int v_count = 0;

    // SEUILS DE DENSITÉ
    // On garde un seuil bas pour capter les lignes fines de l'image 1
    // La vraie filtration se fera via is_line_solid
    int thresh_h = grid_rect.w * 0.10;
    int thresh_v = grid_rect.h * 0.10;

    // --- ANALYSE HORIZONTALE ---
    int start_y = -1;
    for (int y = 0; y < grid_rect.h; y++) {
        // Condition 1: Densité suffisante
        // Condition 2: Est-ce vraiment une ligne solide et pas du texte ?
        if (h_proj[y] > thresh_h && is_line_solid(img, y, 1, grid_rect)) {
            if (start_y == -1) start_y = y; // Début bloc
        } else {
            if (start_y != -1) {
                // Fin bloc, on prend le milieu
                int center = start_y + (y - 1 - start_y) / 2;
                // Filtrage doublons (si lignes très proches < 10px)
                if (h_count == 0 || (center - h_lines[h_count-1] > 15)) {
                    if (h_count < MAX_LINES) h_lines[h_count++] = center;
                }
                start_y = -1;
            }
        }
    }
    // Dernier check bord bas
    if (start_y != -1 && h_count < MAX_LINES) {
        int center = start_y + (grid_rect.h - 1 - start_y) / 2;
        if (h_count == 0 || (center - h_lines[h_count-1] > 15))
            h_lines[h_count++] = center;
    }

    // --- ANALYSE VERTICALE ---
    int start_x = -1;
    for (int x = 0; x < grid_rect.w; x++) {
        // Ignorer marge gauche (bruit)
        if (x < 5) continue;

        if (v_proj[x] > thresh_v && is_line_solid(img, x, 0, grid_rect)) {
            if (start_x == -1) start_x = x;
        } else {
            if (start_x != -1) {
                int center = start_x + (x - 1 - start_x) / 2;
                // Filtrage doublons
                if (v_count == 0 || (center - v_lines[v_count-1] > 15)) {
                    if (v_count < MAX_LINES) v_lines[v_count++] = center;
                }
                start_x = -1;
            }
        }
    }
    if (start_x != -1 && v_count < MAX_LINES) {
        int center = start_x + (grid_rect.w - 1 - start_x) / 2;
        if (v_count == 0 || (center - v_lines[v_count-1] > 15))
            v_lines[v_count++] = center;
    }

    printf("DEBUG: Lignes FILTREES detectees -> H: %d, V: %d\n", h_count, v_count);

    // --- DECOUPAGE ---
    char filename[512];
    int cells_saved = 0;
   
    // Offset pour éviter de voir la grille dans la case
    // 2 pixels suffisent généralement
    int offset = 2;

    for (int i = 0; i < h_count - 1; i++) {
        for (int j = 0; j < v_count - 1; j++) {
            SDL_Rect cell;
           
            cell.x = grid_rect.x + v_lines[j] + offset;
            cell.y = grid_rect.y + h_lines[i] + offset;
           
            // Calculer la largeur dispo jusqu'à la prochaine ligne
            int w_available = (v_lines[j+1] - v_lines[j]);
            int h_available = (h_lines[i+1] - h_lines[i]);
           
            cell.w = w_available - (2 * offset);
            cell.h = h_available - (2 * offset);

            // Sécurité : taille minimale d'une case (ex: 15x15)
            // Si c'est trop petit, c'est probablement un reste de bug de détection
            if (cell.w < 15 || cell.h < 15) continue;

            // Sécurité bornes image
            if (cell.x + cell.w > img->w) cell.w = img->w - cell.x;
            if (cell.y + cell.h > img->h) cell.h = img->h - cell.y;

            sprintf(filename, "%s/cell_%02d_%02d.png", output_folder, i, j);
            save_sub_image(img, cell, filename);
            cells_saved++;
        }
    }
    printf("DEBUG: %d cellules sauvegardees.\n", cells_saved);

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
