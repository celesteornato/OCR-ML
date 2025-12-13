#include "grayscale.h"
#include <err.h>
#include <neural.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(void)
{
    printf("Neural: Finds the solution to an XNOR expression through a neural "
           "network\n"

           "Usage: neural t|(l <file>)\n"
           "\tt: Train network and save it to weights.bin\n"
           "\tl: Load saved network from weights.bin\n");
}

static bool is_valid_arg(const char str[static 2])
{
    return (str[0] == 't' || str[0] == 'l' || str[0] == 's') && str[1] == '\0';
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error: Wrong argument count\n");
        print_usage();
        return 1;
    }
    if (!is_valid_arg(argv[1]))
    {
        printf("Error: bad parameter!");
        print_usage();
        return 1;
    }

    struct neural_network nn = {0};
    if (argv[1][0] == 't')
    {
        neural_train(&nn);
        neural_save_weights(&nn, "weights.bin");
        return 0;
    }

    if (argc != 3)
    {
        printf("Error: Wrong argument count\n");
        print_usage();
        return 1;
    }

    if (argv[1][0] == 's')
    {
        uint_fast8_t arr[32 * 32] = {0};
        path_to_bytes(argv[2], arr, 32, 32);
        for (size_t y = 0; y < 32; ++y)
        {
            for (size_t x = 0; x < 32; ++x)
            {
                (void)putchar(arr[x + (32 * y)] ? '@' : ' ');
            }
            (void)putchar('\n');
        }
        return 0;
    }

    neural_load_weights(&nn, "weights.bin");

    // char path[128] = {0};
    char res = neural_find_logic(&nn, argv[2]);
    printf("Result:%c (%f)\n\n", res, nn.output[res - 'a']);
}
