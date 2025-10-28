#include <math.h>
#include <matrix.h>
#include <neural.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define countof(ARR) (sizeof(ARR) / sizeof(*(ARR)))

static double sigmoid(double x) { return x / (1 + exp(-x)); }
static double dsigmoid(double x) { return x * (1 - x); }
static void nn_process_logic(struct neural_network *nn, bool a, bool b) {
  nn->input[0] = a;
  nn->input[1] = b;

  // Clear out the old values, if there were any
  for (size_t i = 0; i < LAYER1_SIZE; ++i) {
    nn->layer1[i] = nn->layer1_biases[i];
  }
  for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
    nn->output[i] = nn->output_biases[i];
  }

  // We then compute the product
  for (size_t i = 0; i < LAYER1_SIZE; ++i) {
    nn->layer1[i] = line_dot(nn->input, nn->layer1_weights[i], INPUT_SIZE);
  }
  line_map(nn->layer1, LAYER1_SIZE, sigmoid);
  for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
    nn->output[i] = line_dot(nn->layer1, nn->output_weights[i], LAYER1_SIZE);
  }
  line_map(nn->output, OUTPUT_SIZE, sigmoid);
}

bool neural_find_logic(struct neural_network *nn, bool a, bool b) {
  nn_process_logic(nn, a, b);
  return nn->output[0] + nn->output_biases[0] > 0.5;
}

static double rnd(void) {
  return ((double)rand()/(double)(RAND_MAX));
}

void train(struct neural_network *nn) {
  double learning_rate = 1.0;
  for (size_t i = 0; i < LAYER1_SIZE; ++i) {
    nn->layer1_biases[i] = rnd();
    nn->layer1[i] = rnd();
    for (size_t j = 0; j < INPUT_SIZE; ++j) {
      nn->layer1_weights[i][j] = rnd();
    }

  }
  for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
    nn->output_biases[i] = rnd();
    nn->output[i] = rnd();
    for (size_t j = 0; j < LAYER1_SIZE; ++j) {
      nn->output_weights[i][j] = rnd();
    }
  }

  for (size_t i = 0; i < TRAIN_ITERATIONS; ++i) {
    bool a = (bool)(rand() % 2);
    bool b = (bool)(rand() % 2);

    // Even if our output is just one value, we put it in an array for
    // size-genericity
    double result[] = {((!a && !b) || (a && b)) ? 1.0 : 0};
    nn_process_logic(nn, a, b);

    // Back-propagation algorithm to calculate the error
    double output_delta[OUTPUT_SIZE] = {0};
    for (int j = 0; j < OUTPUT_SIZE; ++j) {
      double error = result[j] - nn->output[j];
      output_delta[j] = error * dsigmoid(nn->output[j]);
    }
    double layer1_delta[LAYER1_SIZE] = {0};
    for (int j = 0; j < LAYER1_SIZE; ++j) {
      double error = 0;
      for (int k = 0; k < OUTPUT_SIZE; ++k) {
        error += output_delta[k] * nn->output_weights[k][j];
      }
      layer1_delta[j] = error * dsigmoid(nn->layer1[j]);
    }

    // Apply the correction factor
    for (int j = 0; j < OUTPUT_SIZE; ++j) {
      nn->output_biases[j] += (output_delta[j] * learning_rate);
      for (int k = 0; k < LAYER1_SIZE; ++k) {
        nn->output_weights[j][k] +=
            nn->layer1[k] * output_delta[j] * learning_rate;
      }
    }
    for (int j = 0; j < LAYER1_SIZE; ++j) {
      nn->layer1_biases[j] += (layer1_delta[j] * learning_rate);
      for (int k = 0; k < INPUT_SIZE; ++k) {
        nn->layer1_weights[j][k] +=
            nn->input[k] * layer1_delta[j] * learning_rate;
      }
    }
  }
}
