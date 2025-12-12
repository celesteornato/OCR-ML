#include <matrix.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>

void shuffle(uint_fast8_t array[static 1], size_t count) {
  for (size_t i = 0; i < count - 1; i++) {
    size_t j = i + ((size_t)random() % (count - i));
    uint_fast8_t temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

size_t max_i(const double a[restrict static 1], size_t n)
{
    size_t idx_max = 0;
    double max = -1;

    for (size_t i = 0; i < n; ++i)
    {
        if (a[i] > max)
        {
            max = a[i];
            idx_max = i;
        }
    }

    return idx_max;
}

double line_dot(const double a[restrict static 1],
                const double b[restrict static 1], size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += a[i] * b[i];
    }
    return sum;
}

double line_dot8(const uint_fast8_t a[restrict static 1],
                 const double b[restrict static 1], size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += (double)a[i] * b[i];
    }
    return sum;
}

void line_map(double a[restrict static 1], size_t n, double (*f)(double))
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] = f(a[i]);
    }
}

void line_subi(double a[restrict static 1], const double b[restrict static 1],
               size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] -= b[i];
    }
}
