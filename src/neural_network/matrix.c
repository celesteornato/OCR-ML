#include <matrix.h>

double line_dot(const double a[static restrict 1],
                const double b[static restrict 1], size_t n) {
  double sum = 0;
  for (size_t i = 0; i < n; ++i) {
    sum += a[i] * b[i];
  }
  return sum;
}

void line_add(const double a[static restrict 1],
              const double b[static restrict 1], double out[static restrict 1],
              size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out[i] = a[i] + b[i];
  }
}

void line_sub(const double a[static restrict 1],
              const double b[static restrict 1], double out[static restrict 1],
              size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out[i] = a[i] - b[i];
  }
}

void line_map(double a[static restrict 1], size_t n, double (*f)(double)) {
  for (size_t i = 0; i < n; ++i) {
    a[i] = f(a[i]);
  }
}
