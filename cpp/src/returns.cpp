#include "riskcore/returns.hpp"

#include <stdexcept>

namespace rc {

Matrix simple_returns(const Matrix& prices, std::size_t assets) {
    const std::size_t n = assets == 0 ? prices.cols : assets;
    if (n > prices.cols || prices.rows < 2) throw std::invalid_argument("simple_returns: not enough data");
    Matrix r(prices.rows - 1, n);
    for (std::size_t t = 1; t < prices.rows; ++t)
        for (std::size_t j = 0; j < n; ++j) r(t - 1, j) = prices(t, j) / prices(t - 1, j) - 1.0;
    return r;
}

Vector portfolio_returns(const Matrix& returns, const Vector& weights) {
    if (weights.size() != returns.cols) throw std::invalid_argument("portfolio_returns: one weight per asset");
    return multiply(returns, weights);
}

Vector equal_weights(std::size_t n) { return Vector(n, 1.0 / static_cast<double>(n)); }

}  // namespace rc
