#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <grayscale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline uint8_t set_bit(int n, uint8_t byte)
{
    return (uint8_t)(byte | (1 << n));
}
static inline uint8_t clear_bit(int n, uint8_t byte)
{
    return (uint8_t)(byte & (~(1 << n)));
}
static inline uint8_t change_bit(int n, uint8_t byte, int val)
{
    if (val == 0)
    {
        return clear_bit(n, byte);
    }
    return set_bit(n, byte);
}

SDL_Surface *grayscale(SDL_Surface *src)
{
    // Our approach will be to modify this surface
    SDL_Surface *gray =
        SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0);
    if (!gray)
    {
        return NULL;
    }
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    // The RGB888 format is analogous to a uint32_t
    uint32_t *pixels = gray->pixels;

    for (int y = 0; y < gray->h; y++)
    {
        for (int x = 0; x < gray->w; x++)
        {
            uint32_t pixel = pixels[(y * gray->w) + x];

            // get color of the pixel with RGB code
            SDL_GetRGB(pixel, gray->format, &r, &g, &b);
            // compute the gray
            uint8_t val = (uint8_t)((0.299 * r) + (0.587 * g) + (0.114 * b));
            // write new pixel over old one in grey
            pixels[(y * gray->w) + x] = SDL_MapRGB(gray->format, val, val, val);
        }
    }
    return gray; // return new image in grey
}

uint8_t get_threshold(const SDL_Surface *gray)
{
    uint64_t histogram[256] = {0};

    uint32_t *pixels = (uint32_t *)gray->pixels;
    uint64_t total_pixels = gray->w * gray->h;

    for (int i = 0; i < total_pixels; i++)
    {
        uint8_t r, g, b;
        // Since it's grayscale, r=g=b. We just need one channel.
        SDL_GetRGB(pixels[i], gray->format, &r, &g, &b);
        histogram[r]++;
    }

    // Otsu's Algorithm Wsh
    double sum = 0;
    for (int i = 0; i < 256; i++)
    {
        sum += i * histogram[i];
    }

    double sumB = 0; // Sum of background intensity
    int wB = 0;      // Weight (count) of background
    int wF = 0;      // Weight (count) of foreground

    double varMax = 0;
    int threshold = 0;

    for (int t = 0; t < 256; t++)
    {

        wB += histogram[t];
        if (wB == 0)
            continue;

        wF = total_pixels - wB;
        if (wF == 0)
            break; // All pixels are processed

        sumB += (double)(t * histogram[t]);

        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;

        double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax)
        {
            varMax = varBetween;
            threshold = t;
        }
    }

    printf("Otsu Threshold calculated: %d\n", threshold); // Debug print
    return (uint8_t)threshold;
}

SDL_Surface *apply_threshold(SDL_Surface *src, uint8_t threshold)
{
    SDL_Surface *bnw = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB888, 0);
    if (!bnw)
    {
        return NULL;
    }

    uint32_t *pixels = (uint32_t *)bnw->pixels;
    for (int y = 0; y < bnw->h; y++)
    {
        for (int x = 0; x < bnw->w; x++)
        {
            uint32_t pixel = pixels[(y * bnw->w) + x];
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            SDL_GetRGB(pixel, bnw->format, &r, &g, &b);

            // Again, we expect a grayscale so r = g = b
            uint8_t out = r > threshold ? 255 : 0;
            pixels[(y * bnw->w) + x] = SDL_MapRGB(bnw->format, out, out, out);
        }
    }
    return bnw;
}

int path_to_bitmap(const char path[restrict static 1],
                   uint8_t bitmap[restrict static 1], int h, int w)
{
    (void)bitmap;
    SDL_Surface *img = SDL_LoadBMP(path);

    if (img == NULL)
    {
        printf("Path to Bitmap : Failed to load image: %s from path %s\n",
               SDL_GetError(), path);
        return 0;
    }

    if ((img->h != h) || (img->w / 8 != w))
    {
        printf("Path to Bitmap : image isn't of resolution %dx%d", h * 8,
               w * 8);
        SDL_FreeSurface(img);
        return 0;
    }

    SDL_Surface *gray = grayscale(img);
    SDL_Surface *bnw = apply_threshold(gray, 127);

    uint32_t *pixels = bnw->pixels;
    for (int y = 0; y < bnw->h; y++)
    {
        for (int x = 0; x < bnw->w; x++)
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;

            SDL_GetRGB(pixels[((y * (bnw->w)) + x)], bnw->format, &r, &g, &b);
            bitmap[(x / 8) + (y * w)] =
                change_bit(7 - (x % 8), bitmap[(x / 8) + (y * w)], !r);
        }
    }

    SDL_FreeSurface(bnw);
    SDL_FreeSurface(gray);
    SDL_FreeSurface(img);

    return 1;
}
