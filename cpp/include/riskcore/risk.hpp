// One-day value-at-risk and expected shortfall of a return series, by several methods.
// Both are reported as positive numbers: a value-at-risk of 0.02 is a loss of 2%.
#pragma once

#include <string>
#include <vector>

#include "riskcore/matrix.hpp"
#include "riskcore/volatility.hpp"

namespace rc {

struct RiskEstimate {
    double var = 0.0;
    double es = 0.0;
};

// `r` points to the `n` most recent returns, oldest first. `level` is the confidence level, for example 0.99.
RiskEstimate historical_risk(const double* r, std::size_t n, double level);
RiskEstimate normal_risk(const double* r, std::size_t n, double level);
RiskEstimate cornish_fisher_risk(const double* r, std::size_t n, double level);
RiskEstimate ewma_normal_risk(const double* r, std::size_t n, double level, double lambda);

// Filtered historical simulation: returns are divided by their volatility forecast,
// the empirical quantile of these residuals is scaled by the forecast for the next day.
RiskEstimate filtered_historical_risk(const double* r, std::size_t n, double level, const Vector& variance_path);

RiskEstimate garch_normal_risk(const Garch& model, const double* r, std::size_t n, double level);

// Normal value-at-risk and expected shortfall for a given mean and volatility.
RiskEstimate normal_risk_from_moments(double mean, double volatility, double level);

const std::vector<std::string>& method_names();

// Dispatch by index into `method_names()`. `garch` is used by the GARCH methods only and may be refitted by the caller as rarely as it likes.
RiskEstimate estimate(std::size_t method, const double* r, std::size_t n, double level, double lambda, const Garch& garch);

}  // namespace rc
