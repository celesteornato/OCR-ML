#include <matrix.h>

void line_col_prod(const double a[restrict static 1], const double b[restrict static 1],
                   double out[restrict static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a[i] * b[i];
    }
}

double line_dot(const double a[restrict static 1], const double b[restrict static 1], size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += a[i] * b[i];
    }
    return sum;
}

void line_add(const double a[restrict static 1], const double b[restrict static 1],
              double out[restrict static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a[i] + b[i];
    }
}

void line_sub(const double a[restrict static 1], const double b[restrict static 1],
              double out[restrict static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a[i] - b[i];
    }
}

void line_map(double a[restrict static 1], size_t n, double (*f)(double))
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] = f(a[i]);
    }
}

void line_addi(double a[restrict static 1], const double b[restrict static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] += b[i];
    }
}

void line_subi(double a[restrict static 1], const double b[restrict static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] -= b[i];
    }
}
void line_col_prodi(double a[static 1], const double b[static 1], size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] *= b[i];
    }
}

void line_muli(double a[static 1], double x, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] *= x;
    }
}
