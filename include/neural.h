#ifndef NEURAL_H
#define NEURAL_H

#include <stdbool.h>
#include <stdint.h>

enum {
  INPUT_SIZE = 2,
  LAYER1_SIZE = 10,
  OUTPUT_SIZE = 1,
  TRAIN_ITERATIONS = 20000
};

struct neural_network {
  double input[INPUT_SIZE];

  double layer1[LAYER1_SIZE];
  double output[OUTPUT_SIZE];

  double layer1_biases[LAYER1_SIZE];
  double output_biases[OUTPUT_SIZE];

  // layer1_weights[i][j] : weight that the jth input is given by the ith layer1
  double layer1_weights[LAYER1_SIZE][INPUT_SIZE];
  double output_weights[OUTPUT_SIZE][LAYER1_SIZE];
};

/* Main function for the user, from an initialised network and a bool a and b,
 * we return (!a && !b) || (a && b) */
bool neural_find_logic(struct neural_network *nn, bool a, bool b);

/* Adjusts the weights and biases until it finds the optimal value */
void train(struct neural_network *);

#endif
