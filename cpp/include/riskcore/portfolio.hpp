// Portfolio variance, its decomposition by asset, and minimum-variance weights.
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

double portfolio_variance(const Vector& weights, const Matrix& covariance);

// Euler decomposition of the volatility: contribution i = w_i (C w)_i / volatility. The contributions sum to the volatility.
Vector risk_contributions(const Vector& weights, const Matrix& covariance);

// Fully invested minimum-variance weights C^-1 1 / (1' C^-1 1); short positions allowed.
// Returns false if the covariance matrix is not positive definite.
bool min_variance_weights(const Matrix& covariance, Vector& weights);

// Sum of absolute weights.
double gross_exposure(const Vector& weights);

// Half the sum of absolute weight changes.
double turnover(const Vector& before, const Vector& after);

}  // namespace rc
