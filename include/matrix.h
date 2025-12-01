#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
double line_dot(const double[restrict static 1], const double[restrict static 1], size_t);

// Product of a matrix and of a column vector
void line_col_prod(const double[restrict static 1], const double[restrict static 1],
                   double[restrict static 1], size_t n);
void line_add(const double[restrict static 1], const double[restrict static 1],
              double[restrict static 1], size_t);
void line_sub(const double[restrict static 1], const double[restrict static 1],
              double[restrict static 1], size_t);
void line_map(double[restrict static 1], size_t, double (*)(double));

void line_addi(double[restrict static 1], const double[restrict static 1], size_t);
void line_subi(double[restrict static 1], const double[restrict static 1], size_t);
void line_col_prodi(double[static 1], const double[static 1], size_t);
void line_muli(double[static 1], double, size_t);

#endif
