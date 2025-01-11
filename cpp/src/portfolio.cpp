#include "riskcore/portfolio.hpp"

#include <cmath>

#include "riskcore/linalg.hpp"

namespace rc {

double portfolio_variance(const Vector& weights, const Matrix& covariance) { return dot(weights, multiply(covariance, weights)); }

Vector risk_contributions(const Vector& weights, const Matrix& covariance) {
    Vector marginal = multiply(covariance, weights);
    const double vol = std::sqrt(dot(weights, marginal));
    for (std::size_t i = 0; i < weights.size(); ++i) marginal[i] = weights[i] * marginal[i] / vol;
    return marginal;
}

bool min_variance_weights(const Matrix& covariance, Vector& weights) {
    Matrix lower;
    if (!cholesky(covariance, lower)) return false;
    weights = cholesky_solve(lower, Vector(covariance.rows, 1.0));
    double total = 0.0;
    for (double w : weights) total += w;
    for (double& w : weights) w /= total;
    return true;
}

double gross_exposure(const Vector& weights) {
    double s = 0.0;
    for (double w : weights) s += std::fabs(w);
    return s;
}

double turnover(const Vector& before, const Vector& after) {
    double s = 0.0;
    for (std::size_t i = 0; i < before.size(); ++i) s += std::fabs(after[i] - before[i]);
    return 0.5 * s;
}

}  // namespace rc
