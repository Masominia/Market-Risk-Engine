#include "riskcore/linalg.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace rc {

bool cholesky(const Matrix& a, Matrix& lower) {
    if (a.rows != a.cols) throw std::invalid_argument("cholesky: matrix is not square");
    const std::size_t n = a.rows;
    lower = Matrix(n, n);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j <= i; ++j) {
            double s = a(i, j);
            for (std::size_t k = 0; k < j; ++k) s -= lower(i, k) * lower(j, k);
            if (i == j) {
                if (!(s > 0.0)) return false;
                lower(i, i) = std::sqrt(s);
            } else {
                lower(i, j) = s / lower(j, j);
            }
        }
    return true;
}

Vector cholesky_solve(const Matrix& lower, const Vector& b) {
    const std::size_t n = lower.rows;
    Vector y(n), x(n);
    for (std::size_t i = 0; i < n; ++i) {
        double s = b[i];
        for (std::size_t k = 0; k < i; ++k) s -= lower(i, k) * y[k];
        y[i] = s / lower(i, i);
    }
    for (std::size_t ii = n; ii-- > 0;) {
        double s = y[ii];
        for (std::size_t k = ii + 1; k < n; ++k) s -= lower(k, ii) * x[k];
        x[ii] = s / lower(ii, ii);
    }
    return x;
}

Vector symmetric_eigenvalues(Matrix a, Matrix* vectors) {
    if (a.rows != a.cols) throw std::invalid_argument("symmetric_eigenvalues: matrix is not square");
    const std::size_t n = a.rows;
    Matrix v = identity(n);
    for (int sweep = 0; sweep < 100; ++sweep) {
        double off = 0.0, diag = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            diag += a(i, i) * a(i, i);
            for (std::size_t j = i + 1; j < n; ++j) off += a(i, j) * a(i, j);
        }
        if (off <= 1e-30 * (diag + off) || off == 0.0) break;
        for (std::size_t p = 0; p + 1 < n; ++p)
            for (std::size_t q = p + 1; q < n; ++q) {
                if (a(p, q) == 0.0) continue;
                const double theta = (a(q, q) - a(p, p)) / (2.0 * a(p, q));
                const double t = (theta >= 0.0 ? 1.0 : -1.0) / (std::fabs(theta) + std::sqrt(theta * theta + 1.0));
                const double c = 1.0 / std::sqrt(t * t + 1.0), s = t * c;
                for (std::size_t k = 0; k < n; ++k) {
                    const double akp = a(k, p), akq = a(k, q);
                    a(k, p) = c * akp - s * akq;
                    a(k, q) = s * akp + c * akq;
                }
                for (std::size_t k = 0; k < n; ++k) {
                    const double apk = a(p, k), aqk = a(q, k);
                    a(p, k) = c * apk - s * aqk;
                    a(q, k) = s * apk + c * aqk;
                }
                if (vectors)
                    for (std::size_t k = 0; k < n; ++k) {
                        const double vkp = v(k, p), vkq = v(k, q);
                        v(k, p) = c * vkp - s * vkq;
                        v(k, q) = s * vkp + c * vkq;
                    }
            }
    }
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](std::size_t i, std::size_t j) { return a(i, i) < a(j, j); });
    Vector values(n);
    for (std::size_t i = 0; i < n; ++i) values[i] = a(order[i], order[i]);
    if (vectors) {
        *vectors = Matrix(n, n);
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t k = 0; k < n; ++k) (*vectors)(k, i) = v(k, order[i]);
    }
    return values;
}

double condition_number(const Matrix& a) {
    const Vector e = symmetric_eigenvalues(a);
    if (e.empty() || !(e.front() > 0.0)) return std::numeric_limits<double>::infinity();
    return e.back() / e.front();
}

}  // namespace rc
