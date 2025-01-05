// Dense row-major matrix and the few operations the library needs.
#pragma once

#include <cstddef>
#include <vector>

namespace rc {

using Vector = std::vector<double>;

struct Matrix {
    std::size_t rows = 0, cols = 0;
    std::vector<double> a;

    Matrix() = default;
    Matrix(std::size_t r, std::size_t c, double fill = 0.0) : rows(r), cols(c), a(r * c, fill) {}

    double& operator()(std::size_t i, std::size_t j) { return a[i * cols + j]; }
    double operator()(std::size_t i, std::size_t j) const { return a[i * cols + j]; }
    const double* row(std::size_t i) const { return a.data() + i * cols; }
};

Matrix identity(std::size_t n);
Matrix transpose(const Matrix& m);
Matrix multiply(const Matrix& x, const Matrix& y);
Vector multiply(const Matrix& m, const Vector& v);
double dot(const Vector& x, const Vector& y);
double trace(const Matrix& m);
double frobenius_squared(const Matrix& m);

}  // namespace rc
