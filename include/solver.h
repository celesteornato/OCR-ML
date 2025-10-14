#ifndef SOLVER_H_
#define SOLVER_H_

#define MAX_SIZE 100

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
by the int i and j). If the word is found then we change the coordinates of
the last char coordinates's. Nothing,otherwise. */
void go_directions(const char grid[MAX_SIZE][MAX_SIZE], int index, const char *word, int rows,
                   int cols, int x, int y, int i, int j);

/* Function that finds the word, if so it updates the coordinates of the first
char of our word to the good one (The last coordinates are handleded by go_directions). Otherwise,
nothing */
void search_the_word(const char grid[MAX_SIZE][MAX_SIZE], int rows, int cols, const char *word);

#endif // SOLVER_H_
