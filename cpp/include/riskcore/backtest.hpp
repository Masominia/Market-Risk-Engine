// Statistical tests of a value-at-risk and expected shortfall series against realised losses.
#pragma once

#include <vector>

#include "riskcore/matrix.hpp"

namespace rc {

struct TestResult {
    double statistic = 0.0;
    double p_value = 1.0;
};

// Kupiec proportion-of-failures test: are `exceedances` out of `days` compatible with probability `p`?
TestResult kupiec(std::size_t days, std::size_t exceedances, double p);

// Christoffersen test that exceedances do not cluster (first-order Markov alternative).
TestResult christoffersen_independence(const std::vector<int>& hits);

// Both together: correct frequency and no clustering.
TestResult conditional_coverage(const std::vector<int>& hits, double p);

struct ShortfallCheck {
    // Sum of loss / ES over exceedance days, divided by the expected number of exceedances. One for a correct model;
    // above one, the model underestimates the frequency of exceedances, their size, or both.
    double ratio = 0.0;
    // Mean of loss / ES over exceedance days only: the size of the exceedances alone. One for a correct model.
    double severity = 0.0;
    std::size_t exceedances = 0;
};

// `losses`, `var` and `es` are aligned day by day; `p` is the tail probability of the value-at-risk level.
ShortfallCheck shortfall_check(const Vector& losses, const Vector& var, const Vector& es, double p);

std::vector<int> exceedance_hits(const Vector& losses, const Vector& var);

}  // namespace rc
