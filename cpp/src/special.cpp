#include "riskcore/special.hpp"

#include <cmath>
#include <limits>

namespace rc {

double gamma_p(double a, double x) {
    if (!(a > 0.0) || x < 0.0) return std::numeric_limits<double>::quiet_NaN();
    if (x == 0.0) return 0.0;
    const double log_prefactor = a * std::log(x) - x - std::lgamma(a);
    if (x < a + 1.0) {
        // series
        double term = 1.0 / a, sum = term, n = a;
        for (int i = 0; i < 1000; ++i) {
            n += 1.0;
            term *= x / n;
            sum += term;
            if (std::fabs(term) < std::fabs(sum) * 1e-16) break;
        }
        return sum * std::exp(log_prefactor);
    }
    // continued fraction for the upper function (modified Lentz)
    const double tiny = 1e-300;
    double b = x + 1.0 - a, c = 1.0 / tiny, d = 1.0 / b, h = d;
    for (int i = 1; i < 1000; ++i) {
        const double an = -static_cast<double>(i) * (static_cast<double>(i) - a);
        b += 2.0;
        d = an * d + b;
        if (std::fabs(d) < tiny) d = tiny;
        c = b + an / c;
        if (std::fabs(c) < tiny) c = tiny;
        d = 1.0 / d;
        const double delta = d * c;
        h *= delta;
        if (std::fabs(delta - 1.0) < 1e-16) break;
    }
    return 1.0 - std::exp(log_prefactor) * h;
}

double chi2_survival(double x, double dof) {
    if (x <= 0.0) return 1.0;
    return 1.0 - gamma_p(0.5 * dof, 0.5 * x);
}

}  // namespace rc
