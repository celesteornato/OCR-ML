#ifndef GRID_EXTRACTOR_H
#define GRID_EXTRACTOR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/**
 * Main function to process the OCR image.
 * * 1. Loads the image.
 * 2. Separates the Grid from the Word List (Left/Right split).
 * 3. Saves crops of the Grid and List.
 * 4. Detects cells inside the grid and saves them as individual images.
 * * @param input_path Path to the source image (e.g., "assets/level_1.png")
 * @param output_folder Folder where pngs will be saved (e.g., "output")
 */
void extract_grid_data(const char *input_path, const char *output_folder);

#endif