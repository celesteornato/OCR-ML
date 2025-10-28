#ifndef SOLVER_H
#define SOLVER_H

#include <stddef.h>
enum { MAX_SIZE = 100 };

enum direction_y { DIREC_UP = -1, DIREC_DOWN = 1 };
enum direction_x { DIREC_LEFT = -1, DIREC_RIGHT = 1 };

/* Coordinates of the word searched :
1st : column of the first char
2nd : row of the first char
3rd : column of last char
4th : row of last char */
struct coordinates {
  int start_x;
  int start_y;
  int end_x;
  int end_y;
};

extern struct coordinates word_coordinates;

/* Function called by search_the_word wich test in a given directions (described
by i and j). If the word is found then we change the coordinates of
the last char coordinates's. Nothing,otherwise. */
void go_directions(const char grid[MAX_SIZE][MAX_SIZE], size_t index,
                   const char word[static 1], int rows, int cols, int x, int y,
                   enum direction_x i, enum direction_y j);

/* Function that finds the word, if so it updates the coordinates of the first
char of our word to the good one (The last coordinates are handleded by
go_directions). Otherwise, nothing */
void search_the_word(const char grid[MAX_SIZE][MAX_SIZE], int rows, int cols,
                     const char *word);

/*Call the search of words on each word of the list that we all have to resolve.
 * (En resume) resolve the whole "mots caches"
 */
void resolve(const char *list[], const char grid[MAX_SIZE][MAX_SIZE],
             size_t length, int rows, int cols);

#endif // SOLVER_H
