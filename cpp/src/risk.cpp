#include "riskcore/risk.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "riskcore/normal.hpp"
#include "riskcore/stats.hpp"

namespace rc {

static RiskEstimate from_sorted(const Vector& sorted, double level, double scale) {
    RiskEstimate e;
    e.var = -scale * quantile_sorted(sorted, 1.0 - level);
    e.es = -scale * lower_tail_mean_sorted(sorted, 1.0 - level);
    return e;
}

RiskEstimate historical_risk(const double* r, std::size_t n, double level) {
    Vector s(r, r + n);
    std::sort(s.begin(), s.end());
    return from_sorted(s, level, 1.0);
}

RiskEstimate normal_risk_from_moments(double mean, double volatility, double level) {
    const double z = normal_quantile(level);
    RiskEstimate e;
    e.var = volatility * z - mean;
    e.es = volatility * normal_pdf(z) / (1.0 - level) - mean;
    return e;
}

RiskEstimate normal_risk(const double* r, std::size_t n, double level) { return normal_risk_from_moments(mean(r, n), std_dev(r, n), level); }

RiskEstimate cornish_fisher_risk(const double* r, std::size_t n, double level) {
    const double m = mean(r, n), sd = std_dev(r, n), s = skewness(r, n), k = excess_kurtosis(r, n);
    auto adjusted = [&](double z) { return z + (z * z - 1.0) * s / 6.0 + (z * z * z - 3.0 * z) * k / 24.0 - (2.0 * z * z * z - 5.0 * z) * s * s / 36.0; };
    RiskEstimate e;
    const double z = normal_quantile(1.0 - level);
    e.var = -(m + sd * adjusted(z));
    // expected shortfall: average of the adjusted quantile over the tail, by the midpoint rule
    const int steps = 200;
    double sum = 0.0;
    for (int i = 0; i < steps; ++i) sum += adjusted(normal_quantile((1.0 - level) * (i + 0.5) / steps));
    e.es = -(m + sd * sum / steps);
    // the expansion is not monotone for strong skewness or kurtosis; never report less than the value-at-risk
    e.es = std::max(e.es, e.var);
    return e;
}

RiskEstimate ewma_normal_risk(const double* r, std::size_t n, double level, double lambda) {
    const Vector path = ewma_variance_path(r, n, lambda);
    return normal_risk_from_moments(0.0, std::sqrt(path.back()), level);
}

RiskEstimate filtered_historical_risk(const double* r, std::size_t n, double level, const Vector& variance_path) {
    if (variance_path.size() != n + 1) throw std::invalid_argument("filtered_historical_risk: variance path has the wrong length");
    Vector z(n);
    for (std::size_t t = 0; t < n; ++t) z[t] = r[t] / std::sqrt(variance_path[t]);
    std::sort(z.begin(), z.end());
    return from_sorted(z, level, std::sqrt(variance_path.back()));
}

RiskEstimate garch_normal_risk(const Garch& model, const double* r, std::size_t n, double level) {
    return normal_risk_from_moments(0.0, std::sqrt(garch_variance_path(model, r, n).back()), level);
}

const std::vector<std::string>& method_names() {
    static const std::vector<std::string> names = {"historical", "normal", "cornish_fisher", "ewma_normal", "ewma_filtered", "garch_normal", "garch_filtered"};
    return names;
}

RiskEstimate estimate(std::size_t method, const double* r, std::size_t n, double level, double lambda, const Garch& garch) {
    switch (method) {
        case 0: return historical_risk(r, n, level);
        case 1: return normal_risk(r, n, level);
        case 2: return cornish_fisher_risk(r, n, level);
        case 3: return ewma_normal_risk(r, n, level, lambda);
        case 4: return filtered_historical_risk(r, n, level, ewma_variance_path(r, n, lambda));
        case 5: return garch_normal_risk(garch, r, n, level);
        case 6: return filtered_historical_risk(r, n, level, garch_variance_path(garch, r, n));
        default: throw std::invalid_argument("estimate: unknown method");
    }
}

}  // namespace rc
