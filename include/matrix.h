#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdint.h>

double line_dot(const double[restrict static 1],
                const double[restrict static 1], size_t);

double line_dot8(const uint_fast8_t[restrict static 1],
                 const double[restrict static 1], size_t);

void line_map(double[restrict static 1], size_t, double (*)(double));

void line_subi(double[restrict static 1], const double[restrict static 1],
               size_t);

size_t max_i(const double[restrict static 1], size_t);

#endif
