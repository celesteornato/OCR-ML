#ifndef GRAYSCALE_H
#define GRAYSCALE_H

/*transforme in grayscale (only gray nuances) te colors of the image, return the gray picture*/
SDL_Surface* grayscale(SDL_Surface* src);

/*Compute the threshold of grey off the image by computing the noise of the picture (Otsu method) based on the shape of the image histogram*/
/*returns a threshold usesful for next function, eh oui Virgile*/
Uint8 mid_threshold(SDL_Surface* gray);

/*Transforme the image in black and white, return the black and white picture*/
SDL_Surface* black_n_white(SDL_Surface* src, Uint8 threshold);



#endif 