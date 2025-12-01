#include <err.h>
#include <neural.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(void) {
  printf("Neural: finds the solution to an XNOR expression through a neural "
         "network\n"
         "Usage: neural 0|1 0|1\n");
}

static bool is_bool(const char str[static 2]) {
  return (str[0] == '0' || str[0] == '1') && str[1] == '\0';
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Error: Wrong argument count\n");
    print_usage();
    return 1;
  }
  if (!is_bool(argv[1]) || !is_bool(argv[2])) {
    printf("Error: one of the parameters is not 0|1\n");
    print_usage();
    return 1;
  }
  struct neural_network nn = {0};
  train(&nn);

  bool result = false;
  bool a = false;
  bool b = false;

  b = false, a = true;
  result = neural_find_logic(&nn, a, b);
  printf((a == b) == result ? "Correct, otp at %f\n" : "Incorrect, otp at %f\n", nn.output[0]);
  b = false, a = false;
  result = neural_find_logic(&nn, a, b);
  printf((a == b) == result ? "Correct, otp at %f\n" : "Incorrect, otp at %f\n", nn.output[0]);
  b = true, a = false;
  result = neural_find_logic(&nn, a, b);
  printf((a == b) == result ? "Correct, otp at %f\n" : "Incorrect, otp at %f\n", nn.output[0]);
  b = true, a = true;
  result = neural_find_logic(&nn, a, b);
  printf((a == b) == result ? "Correct, otp at %f\n" : "Incorrect, otp at %f\n", nn.output[0]);
}
