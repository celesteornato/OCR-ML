#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



SDL_Surface* grayscale(SDL_Surface* src)
{
    SDL_Surface* gray = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0); //we create a new surface
    if (!gray)      // wich contains exactly the same pixels,but stocked under 3 octets by pixels : R, G, B
    {
        return NULL; 
    }
    Uint8 r, g, b;
    Uint32* pixels = (Uint32*)gray->pixels; //all pixels of gray cnverted in 32bits because they are in RGB format

    for (int y = 0; y < gray->h; y++) // going through all pixels
    {
        for (int x = 0; x < gray->w; x++)
        {
            Uint32 pixel = pixels[y * gray->w + x]; // get the pixel
            SDL_GetRGB(pixel, gray->format, &r, &g, &b); //get color of the pixel with RGB code
            Uint8 val = (Uint8)(0.299 * r + 0.587 * g + 0.114 * b); // calculus of grey
            pixels[y * gray->w + x] = SDL_MapRGB(gray->format, val, val, val); //write new pixel over old one in grey
        }
    }
    return gray; //return new image in grey

}

Uint8 otsu(SDL_Surface* gray)
{
    Uint32 histogram[256] = {0}; // create historgam
    Uint32* pixels = (Uint32*)gray->pixels; // all pixels of the gray picture
    Uint8 r, g, b;

    for (int y = 0; y < gray->h; y++)
    {   
        for (int x = 0; x < gray->w; x++)
        {
            SDL_GetRGB(pixels[y * gray->w + x], gray->format, &r, &g, &b); //because gray, r == g == b
            histogram[r]++; //add to the histogram the value of gray
        }
    }

}




SDL_Surface* black_n_white(SDL_Surface* src, Uint8 threshold)
{
    SDL_Surface* black_n_white = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0); //we create a new surface
    if(!black_n_white)
    {
         return NULL;
    }

    Uint8 r, g, b;
    Uint32* pixels = (Uint32*)black_n_white->pixels; //same
    for (int y = 0; y < black_n_white->h; y++) // going through all pixels
    {
        for (int x = 0; x < black_n_white->w; x++)
        {
            Uint32 pixel = pixels[y * black_n_white->w + x]; // get the pixel
            SDL_GetRGB(pixel, black_n_white->format, &r, &g, &b); //get color of the pixel with RGB code here r == g == b because grey 
            if(r > threshold)
            {
                pixels[y * black_n_white->w + x] = SDL_MapRGB(black_n_white->format, 255, 255, 255); //in white
            }
            else
            {
                pixels[y * black_n_white->w + x] = SDL_MapRGB(black_n_white->format, 0, 0, 0); //write new pixel over old one in black
            }
        }
    }
    return black_n_white; 
}


int main(int argc, char* argv[])
{
    if (argc != 2) //error
    {
        printf("I could use only one picture ... :(");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* img = SDL_LoadBMP(argv[1]);
    if (!img) //error
    {
        printf("Failed to load image: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface* gray = grayscale(img); //gray
    Uint8 threshold = otsu(gray); //thresold of gray with otsu method
    SDL_Surface* black_n_white = black_n_white(gray, threshold); // final result

    printf("Threshold = %d\n", threshold);

    SDL_SaveBMP(gray, "gray.bmp");
    SDL_SaveBMP(black_n_white, "black_n_white.bmp");

    SDL_FreeSurface(black_n_white);
    SDL_FreeSurface(gray);
    SDL_FreeSurface(img);
    SDL_Quit();

    printf("Saved gray.bmp and black_n_white.bmp\n");
    return 0;
}
