#ifndef NEURAL_H
#define NEURAL_H

#include <stdbool.h>
#include <stdint.h>

#define LEARNING_RATE 0.01
enum {
    INPUT_SIZE = 32 * 32,
    LAYER1_SIZE = 168,
    LAYER2_SIZE = 74,
    OUTPUT_SIZE = 26,

    DATASET_SIZE = 130000,
    DATASET_PER_INPUT = 5000,
    EPOCHS = 2000
};

struct neural_network {
    uint_fast8_t input[INPUT_SIZE];
    double layer1[LAYER1_SIZE];
    double layer2[LAYER2_SIZE];
    double output[OUTPUT_SIZE];

    double layer1_biases[LAYER1_SIZE];
    double layer2_biases[LAYER2_SIZE];
    double output_biases[OUTPUT_SIZE];

    // layer1_weights[j][i] : weight that the ith input is given by the ith
    // layer1
    double layer1_weights[LAYER1_SIZE][INPUT_SIZE];
    double layer2_weights[LAYER2_SIZE][LAYER1_SIZE];
    double output_weights[OUTPUT_SIZE][LAYER2_SIZE];
};

/* Initialises the network by training it from scratch */
void neural_train(struct neural_network *);
/* Initialises the network by training it from a file */
void neural_load_weights(struct neural_network *, const char[static 1]);
/* Writes trained weights to a file */
void neural_save_weights(struct neural_network *, const char[static 1]);

/* Main function for the user */
char neural_find_logic(struct neural_network *nn, const char path[static 1]);

#endif
