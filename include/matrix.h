#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
double line_dot(const double[static restrict 1],
                const double[static restrict 1], size_t);

void line_add(const double[static restrict 1], const double[static restrict 1],
              double[static restrict 1], size_t);

void line_sub(const double[static restrict 1], const double[static restrict 1],
              double[static restrict 1], size_t);

void line_map(double[static restrict 1], size_t, double (*)(double));

#endif
