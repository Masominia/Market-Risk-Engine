// Conditional volatility models for a single return series.
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

// EWMA variance path: out[t] is the variance forecast for return t made from returns before t,
// for t in [begin, end]; out has end - begin + 1 entries, the last one is the forecast for the next day.
// The recursion starts from the sample variance of the window.
Vector ewma_variance_path(const double* r, std::size_t n, double lambda);

struct Garch {
    double omega = 0.0, alpha = 0.0, beta = 0.0;
    double log_likelihood = 0.0;
};

// GARCH(1,1) with normal innovations and zero mean, fitted by maximum likelihood.
// The long-run variance is fixed at the sample variance, leaving alpha and beta to a derivative-free search.
Garch fit_garch(const double* r, std::size_t n);

// Variance path of a GARCH model, same layout as the EWMA path.
Vector garch_variance_path(const Garch& model, const double* r, std::size_t n);

}  // namespace rc
