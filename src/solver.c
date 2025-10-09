#include "solver.h"

/*
Rewind for Virgile (sacre Virgile !):

 y  x
{0, 0} no move
{0, 1} right
{0,-1} left
{1, 0} down
{-1,0} up
{1, 1} down-right
{1,-1} down-left
{-1,1} up-rigt
{-1,-1} up-left
*/

struct Coordinates Word_Coordinates = {-1, -1, -1, -1,};


void go_directions(char grid[MAX_SIZE][MAX_SIZE], int index, char *word, int rows, int cols, int x, int y, int i, int j)
{
    int len = (int)strlen(word);
    while(x >= 0 && y >= 0 && x<cols && y<rows && index< len && grid[y][x] == word[index])
    {
	    x += i;
	    y += j;
	    index++;
    }
    if(index == len)
    {
	    Word_Coordinates.end_x = x-i;
	    Word_Coordinates.end_y = y-j;
	    return;
    }
    return;

}

void search_the_word(char grid[MAX_SIZE][MAX_SIZE], int rows, int cols, char *word)
{
    for(int y = 0; y< rows; y++)
    {
        for(int x = 0; x < cols; x++)
	{
	    if(grid[y][x] != word[0])
	    {
		    continue; //Not the 1st char like wtf why should I care
	    }
	    for(int i = -1; i<= 1; i++) //going up/nothing/down
	    {
		    for(int j = 0; j<= 1; j++) //going left/nothing/right
		    {
			if(i==0 && j==0) //if there's no move quoi like wtf I am not paid to work for nothing
			{ continue; }
			Word_Coordinates.end_x = -1;
			go_directions(grid, 1, word, rows, cols, x+i, y+j, i, j);
			if(Word_Coordinates.end_x != -1) //if it has been find
			{
				Word_Coordinates.start_x = x;
				Word_Coordinates.start_y = y;
				printf("(%d,%d),(%d,%d)", Word_Coordinates.start_x, Word_Coordinates.start_y, Word_Coordinates.end_x, Word_Coordinates.end_y);
				return;
			}
		    }
	    }
	}
    }
    printf("Not found\n");
    return; //no change (sad, we did not find our word)
}


int main(void)
{
    char grid[MAX_SIZE][MAX_SIZE]=
    {
	"DGHEYEUJKQ",
	"FSGDJNHJJD",
	"EYUATTSNDI",
	"KIOLKRLKI",
	"TEFRTOFRTE",
	"ASZXDPHKLS",
	"ETYSGIKNXK",
	"UKGFFCEDED"
    };

    int rows = 8;
    int cols = 10;
    char word[] = "ENTROPIC";

    printf("Searching for %s\n", word);
    search_the_word(grid, rows, cols, word); //should be (5,0),(5,7)

    printf("\n");
    return 0;
}
