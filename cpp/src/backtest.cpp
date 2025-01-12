#include "riskcore/backtest.hpp"

#include <cmath>
#include <limits>

#include "riskcore/special.hpp"

namespace rc {

// x log(x / y) with the convention 0 log 0 = 0
static double xlogy(double count, double prob) { return count > 0.0 ? count * std::log(prob) : 0.0; }

TestResult kupiec(std::size_t days, std::size_t exceedances, double p) {
    const double n = static_cast<double>(days), x = static_cast<double>(exceedances);
    const double rate = x / n;
    const double null = xlogy(n - x, 1.0 - p) + xlogy(x, p);
    const double alt = xlogy(n - x, 1.0 - rate) + xlogy(x, rate);
    TestResult t;
    t.statistic = std::max(0.0, -2.0 * (null - alt));
    t.p_value = chi2_survival(t.statistic, 1.0);
    return t;
}

TestResult christoffersen_independence(const std::vector<int>& hits) {
    double n00 = 0, n01 = 0, n10 = 0, n11 = 0;
    for (std::size_t t = 1; t < hits.size(); ++t) {
        if (hits[t - 1] == 0 && hits[t] == 0) ++n00;
        if (hits[t - 1] == 0 && hits[t] != 0) ++n01;
        if (hits[t - 1] != 0 && hits[t] == 0) ++n10;
        if (hits[t - 1] != 0 && hits[t] != 0) ++n11;
    }
    TestResult t;
    if (n01 + n11 == 0.0 || n10 + n11 == 0.0) return t;  // no exceedance, or nothing follows one: no evidence either way
    const double pi01 = n01 / (n00 + n01), pi11 = n11 / (n10 + n11), pi = (n01 + n11) / (n00 + n01 + n10 + n11);
    const double null = xlogy(n00 + n10, 1.0 - pi) + xlogy(n01 + n11, pi);
    const double alt = xlogy(n00, 1.0 - pi01) + xlogy(n01, pi01) + xlogy(n10, 1.0 - pi11) + xlogy(n11, pi11);
    t.statistic = std::max(0.0, -2.0 * (null - alt));
    t.p_value = chi2_survival(t.statistic, 1.0);
    return t;
}

TestResult conditional_coverage(const std::vector<int>& hits, double p) {
    std::size_t x = 0;
    for (int h : hits) x += h != 0;
    TestResult t;
    t.statistic = kupiec(hits.size(), x, p).statistic + christoffersen_independence(hits).statistic;
    t.p_value = chi2_survival(t.statistic, 2.0);
    return t;
}

std::vector<int> exceedance_hits(const Vector& losses, const Vector& var) {
    std::vector<int> hits(losses.size());
    for (std::size_t t = 0; t < losses.size(); ++t) hits[t] = losses[t] > var[t] ? 1 : 0;
    return hits;
}

ShortfallCheck shortfall_check(const Vector& losses, const Vector& var, const Vector& es, double p) {
    ShortfallCheck c;
    double sum = 0.0;
    for (std::size_t t = 0; t < losses.size(); ++t)
        if (losses[t] > var[t]) {
            sum += losses[t] / es[t];
            ++c.exceedances;
        }
    c.ratio = sum / (static_cast<double>(losses.size()) * p);
    c.severity = c.exceedances > 0 ? sum / static_cast<double>(c.exceedances) : std::numeric_limits<double>::quiet_NaN();
    return c;
}

}  // namespace rc
