// Covariance estimators on a window of the return matrix: rows [begin, end).
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

Matrix sample_covariance(const Matrix& returns, std::size_t begin, std::size_t end);

// Exponentially weighted covariance with zero mean; the newest row has the largest weight.
Matrix ewma_covariance(const Matrix& returns, std::size_t begin, std::size_t end, double lambda);

struct Shrunk {
    Matrix covariance;
    double intensity = 0.0;  // weight of the target, between 0 and 1
};

// Ledoit-Wolf shrinkage of the sample covariance towards a multiple of the identity,
// with the intensity that minimises the expected squared error.
Shrunk ledoit_wolf(const Matrix& returns, std::size_t begin, std::size_t end);

// Keeps the variances, sets the covariances to zero.
Matrix diagonal_part(const Matrix& covariance);

Matrix correlation(const Matrix& covariance);

}  // namespace rc
