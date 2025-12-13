#ifndef GRID_EXTRACTOR_H
#define GRID_EXTRACTOR_H

/**
 * Main function to process the OCR image.
 * 1. Loads the image.
 * 2. Separates the Grid from the Word List (Left/Right split).
 * 3. Saves crops of the Grid and List.
 * 4. Detects cells inside the grid and saves them as individual images.
 */
void extract_grid_data(const char *input_path, const char *output_folder,
                       int *h_count, int *v_count);

#endif
