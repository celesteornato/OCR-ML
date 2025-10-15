#include <solver.h>
#include <stdio.h>
#include <string.h>

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

struct coordinates word_coordinates = {
    -1,
    -1,
    -1,
    -1,
};

void go_directions(const char grid[MAX_SIZE][MAX_SIZE], int index, const char *word, int rows,
                   int cols, int x, int y, int i, int j)
{
    int len = (int)strlen(word);
    while (x >= 0 && y >= 0 && x < cols && y < rows && index < len && grid[y][x] == word[index])
    {
        x += i;
        y += j;
        index++;
    }
    if (index == len)
    {
        word_coordinates.end_x = x - i;
        word_coordinates.end_y = y - j;
    }
}

static void search_the_word(const char grid[MAX_SIZE][MAX_SIZE], int rows, int cols, const char *word)
{
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (grid[y][x] != word[0])
            {
                continue; // Not the 1st char like wtf why should I care
            }
            for (int i = -1; i <= 1; i++) // going up/nothing/down
            {
                for (int j = 0; j <= 1; j++) // going left/nothing/right
                {
                    // if there's no move quoi like wtf I am not paid to work for nothing
                    if (i == 0 && j == 0)
                    {
                        continue;
                    }
                    word_coordinates.end_x = -1;
                    go_directions(grid, 1, word, rows, cols, x + i, y + j, i, j);
                    if (word_coordinates.end_x != -1) // if it has been find
                    {
                        word_coordinates.start_x = x;
                        word_coordinates.start_y = y;
                        printf("(%d,%d),(%d,%d)\n", word_coordinates.start_x,
                               word_coordinates.start_y, word_coordinates.end_x,
                               word_coordinates.end_y);
                        return;
                    }
                }
            }
        }
    }
    printf("Not found\n");
}

void resolve(const char **list[], char grid[MAX_SIZE][MAX_SIZE], size_t length, int rows, int cols)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("Searching for %s\n", list[i]);
        search_the_word(grid, rows, cols, list[i]);
    }
}

int main(void)
{
    char grid[MAX_SIZE][MAX_SIZE] = {"DGHEYEUJKQIDIDIDIDID", "FSING1QJJDAAAAAAAAAA",
                                     "EYUTTFSODIAAAAHAAAAA", "KIORKFLKLAAAAEAAAAAA",
                                     "TEFOTOFRTLAALAAAAAAA", "ASZPDPHKLSOLAAAAAAAA",
                                     "ETYIGIKNXKOAAAAAAAAA", "UKGCFCEDEDAAAAAAAAAA"};

    int rows = 8;
    int cols = 20;
    char word[] = "HELLO";     // should give (2,13),(6,9)
    char word2[] = "ENTROPIC"; // should give (0,3)(7,3)
    char word3[] = "ING1";     // should give (1,2)(1,5)
    const char *list;
    list.add(word);
    list.add(word2);
    list.add(word3);

    resolve(list, grid, 3, rows, cols);

    printf("\n");
    return 0;
}
