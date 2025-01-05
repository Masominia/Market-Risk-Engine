#include "riskcore/matrix.hpp"

#include <stdexcept>

namespace rc {

Matrix identity(std::size_t n) {
    Matrix m(n, n);
    for (std::size_t i = 0; i < n; ++i) m(i, i) = 1.0;
    return m;
}

Matrix transpose(const Matrix& m) {
    Matrix t(m.cols, m.rows);
    for (std::size_t i = 0; i < m.rows; ++i)
        for (std::size_t j = 0; j < m.cols; ++j) t(j, i) = m(i, j);
    return t;
}

Matrix multiply(const Matrix& x, const Matrix& y) {
    if (x.cols != y.rows) throw std::invalid_argument("multiply: shapes do not match");
    Matrix z(x.rows, y.cols);
    for (std::size_t i = 0; i < x.rows; ++i)
        for (std::size_t k = 0; k < x.cols; ++k) {
            const double v = x(i, k);
            if (v == 0.0) continue;
            for (std::size_t j = 0; j < y.cols; ++j) z(i, j) += v * y(k, j);
        }
    return z;
}

Vector multiply(const Matrix& m, const Vector& v) {
    if (m.cols != v.size()) throw std::invalid_argument("multiply: shapes do not match");
    Vector out(m.rows, 0.0);
    for (std::size_t i = 0; i < m.rows; ++i) {
        double s = 0.0;
        for (std::size_t j = 0; j < m.cols; ++j) s += m(i, j) * v[j];
        out[i] = s;
    }
    return out;
}

double dot(const Vector& x, const Vector& y) {
    if (x.size() != y.size()) throw std::invalid_argument("dot: sizes do not match");
    double s = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) s += x[i] * y[i];
    return s;
}

double trace(const Matrix& m) {
    double s = 0.0;
    for (std::size_t i = 0; i < m.rows && i < m.cols; ++i) s += m(i, i);
    return s;
}

double frobenius_squared(const Matrix& m) {
    double s = 0.0;
    for (double v : m.a) s += v * v;
    return s;
}

}  // namespace rc
