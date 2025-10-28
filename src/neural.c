#include <math.h>
#include <stdbool.h>
#include <stddef.h>

static double sigmoid(double x) { return x / (1 + exp(-x)); }

static bool output(double inputs[static 1], double weights[static 1],
                   size_t len) {}
