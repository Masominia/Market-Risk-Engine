// Descriptive statistics on ranges of doubles.
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

double mean(const double* x, std::size_t n);
double variance(const double* x, std::size_t n);  // divisor n - 1
double std_dev(const double* x, std::size_t n);
double skewness(const double* x, std::size_t n);
double excess_kurtosis(const double* x, std::size_t n);

// Empirical quantile with linear interpolation between order statistics; `sorted` must be ascending.
double quantile_sorted(const Vector& sorted, double p);
double quantile(const double* x, std::size_t n, double p);

// Mean of the values at or below the p-quantile of an ascending sample: the lower tail average.
double lower_tail_mean_sorted(const Vector& sorted, double p);

inline double mean(const Vector& x) { return mean(x.data(), x.size()); }
inline double variance(const Vector& x) { return variance(x.data(), x.size()); }
inline double std_dev(const Vector& x) { return std_dev(x.data(), x.size()); }

}  // namespace rc
