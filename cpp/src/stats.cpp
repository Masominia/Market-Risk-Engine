#include "riskcore/stats.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace rc {

double mean(const double* x, std::size_t n) {
    if (n == 0) return std::numeric_limits<double>::quiet_NaN();
    double s = 0.0;
    for (std::size_t i = 0; i < n; ++i) s += x[i];
    return s / static_cast<double>(n);
}

double variance(const double* x, std::size_t n) {
    if (n < 2) return std::numeric_limits<double>::quiet_NaN();
    const double m = mean(x, n);
    double s = 0.0;
    for (std::size_t i = 0; i < n; ++i) s += (x[i] - m) * (x[i] - m);
    return s / static_cast<double>(n - 1);
}

double std_dev(const double* x, std::size_t n) { return std::sqrt(variance(x, n)); }

static double central_moment(const double* x, std::size_t n, int k) {
    const double m = mean(x, n);
    double s = 0.0;
    for (std::size_t i = 0; i < n; ++i) s += std::pow(x[i] - m, k);
    return s / static_cast<double>(n);
}

double skewness(const double* x, std::size_t n) {
    return central_moment(x, n, 3) / std::pow(central_moment(x, n, 2), 1.5);
}

double excess_kurtosis(const double* x, std::size_t n) {
    const double m2 = central_moment(x, n, 2);
    return central_moment(x, n, 4) / (m2 * m2) - 3.0;
}

double quantile_sorted(const Vector& sorted, double p) {
    if (sorted.empty()) return std::numeric_limits<double>::quiet_NaN();
    const double pos = std::clamp(p, 0.0, 1.0) * static_cast<double>(sorted.size() - 1);
    const std::size_t lo = static_cast<std::size_t>(std::floor(pos));
    const std::size_t hi = std::min(lo + 1, sorted.size() - 1);
    return sorted[lo] + (pos - static_cast<double>(lo)) * (sorted[hi] - sorted[lo]);
}

double quantile(const double* x, std::size_t n, double p) {
    Vector s(x, x + n);
    std::sort(s.begin(), s.end());
    return quantile_sorted(s, p);
}

double lower_tail_mean_sorted(const Vector& sorted, double p) {
    // integral of the interpolated quantile function over [0, p], divided by p
    if (sorted.empty()) return std::numeric_limits<double>::quiet_NaN();
    if (sorted.size() == 1) return sorted[0];
    const double span = static_cast<double>(sorted.size() - 1);
    const double pos = std::clamp(p, 1e-12, 1.0) * span;
    const std::size_t full = static_cast<std::size_t>(std::floor(pos));
    double area = 0.0;
    for (std::size_t i = 0; i < full && i + 1 < sorted.size(); ++i) area += 0.5 * (sorted[i] + sorted[i + 1]);
    const double frac = pos - static_cast<double>(full);
    if (frac > 0.0 && full + 1 < sorted.size()) {
        const double end = sorted[full] + frac * (sorted[full + 1] - sorted[full]);
        area += 0.5 * (sorted[full] + end) * frac;
    }
    return area / pos;
}

}  // namespace rc
