#ifndef SOLVER_H
#define SOLVER_H

#include <stdio.h>
#include <string.h>

#define MAX_SIZE 100



struct Coordinates
{
    int start_x;
    int start_y;
    int end_x;
    int end_y;
};
/* 
Coordinates of the word searched :
1st : column of the first char
2nd : row of the first char
3rd : column of last char
4th : row of last char
*/
extern struct Coordinates Word_Coordinates;
//Our word's coordinates

void go_directions(char grid[MAX_SIZE][MAX_SIZE], int index, char *word, int rows, int cols, int x, int y, int i, int j);
/*
Functiun called by search_the_word wich test in a given directions (described
by the int i and j). If the word is found then we change the coordinates of
the last char coordinates's. Nothing,otherwise.
*/
void search_the_word(char grid[MAX_SIZE][MAX_SIZE], int rows, int cols, char *word);
/*
Functiun that finds the word, if so it updates the coordinates of the first
char of our word to the good one (The last coordinates are handleded by
go_directions). Nothing, otherwise
*/

#endif



