// Rolling-window experiments: value-at-risk backtests and minimum-variance portfolios.
#pragma once

#include <string>
#include <vector>

#include "riskcore/backtest.hpp"
#include "riskcore/matrix.hpp"

namespace rc {

struct BacktestConfig {
    std::size_t window = 250;
    double var_level = 0.99;
    double es_level = 0.975;
    double lambda = 0.94;
    std::size_t garch_refit = 21;  // days between two fits of the GARCH parameters
};

struct MethodResult {
    std::string name;
    std::size_t exceedances = 0;
    TestResult kupiec, independence, coverage;
    ShortfallCheck shortfall;
    double mean_var = 0.0;
    Vector var_series;  // at var_level
    Vector es_series;   // at es_level
};

struct BacktestResult {
    Vector losses;  // one per out-of-sample day
    std::vector<MethodResult> methods;
};

// Every day after the first window, each method forecasts the next day's risk from the previous `window` returns.
BacktestResult rolling_backtest(const Vector& returns, const BacktestConfig& config);

struct MinVarResult {
    std::string name;
    double volatility = 0.0;  // annualised, out of sample
    double gross = 0.0;       // mean sum of absolute weights
    double turnover = 0.0;    // mean per rebalance
    double intensity = 0.0;   // mean shrinkage intensity, where it applies
    std::size_t failures = 0; // rebalances at which the covariance matrix could not be inverted
};

const std::vector<std::string>& estimator_names();

// Minimum-variance weights are recomputed every `rebalance` days from the previous `window` days and held in between.
MinVarResult rolling_min_variance(const Matrix& returns, std::size_t window, std::size_t rebalance, std::size_t estimator);

}  // namespace rc
