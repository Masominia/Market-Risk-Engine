#include "riskcore/normal.hpp"

#include <cmath>
#include <limits>

namespace rc {

double normal_pdf(double x) { return 0.3989422804014327 * std::exp(-0.5 * x * x); }

double normal_cdf(double x) { return 0.5 * std::erfc(-x * 0.7071067811865476); }

double normal_quantile(double p) {
    if (!(p > 0.0 && p < 1.0)) return p == 0.0 ? -std::numeric_limits<double>::infinity() : (p == 1.0 ? std::numeric_limits<double>::infinity() : std::numeric_limits<double>::quiet_NaN());
    // rational approximation, then two Newton-type corrections on the exact distribution function
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02, 1.383577518672690e+02, -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02, 6.680131188771972e+01, -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00, -2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00};
    static const double d[] = {7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00, 3.754408661907416e+00};
    double x;
    if (p < 0.02425) {
        const double q = std::sqrt(-2.0 * std::log(p));
        x = (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    } else if (p > 1.0 - 0.02425) {
        const double q = std::sqrt(-2.0 * std::log(1.0 - p));
        x = -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    } else {
        const double q = p - 0.5, r = q * q;
        x = (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q / (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
    }
    for (int i = 0; i < 2; ++i) {
        const double e = normal_cdf(x) - p;
        const double u = e / normal_pdf(x);
        x -= u / (1.0 + 0.5 * x * u);
    }
    return x;
}

}  // namespace rc
