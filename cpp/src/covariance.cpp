#include "riskcore/covariance.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rc {

static Matrix centred(const Matrix& r, std::size_t begin, std::size_t end) {
    if (end > r.rows || end < begin + 2) throw std::invalid_argument("covariance: window too short");
    const std::size_t n = end - begin, p = r.cols;
    Matrix x(n, p);
    for (std::size_t j = 0; j < p; ++j) {
        double m = 0.0;
        for (std::size_t t = begin; t < end; ++t) m += r(t, j);
        m /= static_cast<double>(n);
        for (std::size_t t = begin; t < end; ++t) x(t - begin, j) = r(t, j) - m;
    }
    return x;
}

static Matrix gram(const Matrix& x, double divisor) {
    const std::size_t n = x.rows, p = x.cols;
    Matrix s(p, p);
    for (std::size_t t = 0; t < n; ++t) {
        const double* row = x.row(t);
        for (std::size_t i = 0; i < p; ++i) {
            const double v = row[i];
            double* out = &s.a[i * p];
            for (std::size_t j = i; j < p; ++j) out[j] += v * row[j];
        }
    }
    for (std::size_t i = 0; i < p; ++i)
        for (std::size_t j = i; j < p; ++j) {
            s(i, j) /= divisor;
            s(j, i) = s(i, j);
        }
    return s;
}

Matrix sample_covariance(const Matrix& returns, std::size_t begin, std::size_t end) {
    return gram(centred(returns, begin, end), static_cast<double>(end - begin - 1));
}

Matrix ewma_covariance(const Matrix& returns, std::size_t begin, std::size_t end, double lambda) {
    if (end > returns.rows || end <= begin) throw std::invalid_argument("ewma_covariance: empty window");
    const std::size_t p = returns.cols;
    Matrix s(p, p);
    double weight = 1.0, total = 0.0;
    for (std::size_t t = end; t-- > begin;) {
        const double* row = returns.row(t);
        for (std::size_t i = 0; i < p; ++i)
            for (std::size_t j = i; j < p; ++j) s(i, j) += weight * row[i] * row[j];
        total += weight;
        weight *= lambda;
    }
    for (std::size_t i = 0; i < p; ++i)
        for (std::size_t j = i; j < p; ++j) {
            s(i, j) /= total;
            s(j, i) = s(i, j);
        }
    return s;
}

Shrunk ledoit_wolf(const Matrix& returns, std::size_t begin, std::size_t end) {
    const Matrix x = centred(returns, begin, end);
    const double n = static_cast<double>(x.rows), p = static_cast<double>(x.cols);
    Matrix s = gram(x, n);
    const double mu = trace(s) / p;
    double d2 = frobenius_squared(s) - 2.0 * mu * trace(s) + mu * mu * p;  // ||S - mu I||^2
    d2 /= p;
    // sum over observations of ||x x' - S||^2 = sum (x'x)^2 - n ||S||^2
    double fourth = 0.0;
    for (std::size_t t = 0; t < x.rows; ++t) {
        double q = 0.0;
        const double* row = x.row(t);
        for (std::size_t j = 0; j < x.cols; ++j) q += row[j] * row[j];
        fourth += q * q;
    }
    double b2 = (fourth - n * frobenius_squared(s)) / (n * n) / p;
    b2 = std::clamp(b2, 0.0, d2);
    Shrunk out;
    out.intensity = d2 > 0.0 ? b2 / d2 : 1.0;
    for (std::size_t i = 0; i < s.rows; ++i)
        for (std::size_t j = 0; j < s.cols; ++j) s(i, j) = (1.0 - out.intensity) * s(i, j) + (i == j ? out.intensity * mu : 0.0);
    out.covariance = std::move(s);
    return out;
}

Matrix diagonal_part(const Matrix& covariance) {
    Matrix d(covariance.rows, covariance.cols);
    for (std::size_t i = 0; i < covariance.rows; ++i) d(i, i) = covariance(i, i);
    return d;
}

Matrix correlation(const Matrix& covariance) {
    Matrix c(covariance.rows, covariance.cols);
    for (std::size_t i = 0; i < c.rows; ++i)
        for (std::size_t j = 0; j < c.cols; ++j) c(i, j) = covariance(i, j) / std::sqrt(covariance(i, i) * covariance(j, j));
    return c;
}

}  // namespace rc
