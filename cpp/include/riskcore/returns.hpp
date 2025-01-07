// From prices to returns, and from asset returns to portfolio returns.
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

// Simple returns P(t) / P(t-1) - 1, restricted to the first `assets` columns (all if zero).
Matrix simple_returns(const Matrix& prices, std::size_t assets = 0);

// Return of a portfolio rebalanced to `weights` every day.
Vector portfolio_returns(const Matrix& returns, const Vector& weights);

Vector equal_weights(std::size_t n);

}  // namespace rc
