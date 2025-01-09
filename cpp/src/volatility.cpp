#include "riskcore/volatility.hpp"

#include <cmath>
#include <limits>

#include "riskcore/stats.hpp"

namespace rc {

static double second_moment(const double* r, std::size_t n) {
    double s = 0.0;
    for (std::size_t i = 0; i < n; ++i) s += r[i] * r[i];
    return s / static_cast<double>(n);
}

Vector ewma_variance_path(const double* r, std::size_t n, double lambda) {
    Vector v(n + 1);
    v[0] = second_moment(r, n);
    for (std::size_t t = 0; t < n; ++t) v[t + 1] = lambda * v[t] + (1.0 - lambda) * r[t] * r[t];
    return v;
}

static double garch_likelihood(const double* r, std::size_t n, double target, double alpha, double beta) {
    if (alpha < 0.0 || beta < 0.0 || alpha + beta >= 0.9999) return -std::numeric_limits<double>::infinity();
    const double omega = target * (1.0 - alpha - beta);
    double v = target, ll = 0.0;
    for (std::size_t t = 0; t < n; ++t) {
        ll -= 0.5 * (std::log(v) + r[t] * r[t] / v);
        v = omega + alpha * r[t] * r[t] + beta * v;
    }
    return ll;
}

Garch fit_garch(const double* r, std::size_t n) {
    const double target = second_moment(r, n);
    // coarse grid, then a shrinking pattern search around the best point
    double best_a = 0.05, best_b = 0.90, best = -std::numeric_limits<double>::infinity();
    for (int i = 0; i <= 12; ++i)
        for (int j = 0; j <= 19; ++j) {
            const double a = 0.01 + 0.025 * i, b = 0.02 + 0.05 * j;
            const double ll = garch_likelihood(r, n, target, a, b);
            if (ll > best) best = ll, best_a = a, best_b = b;
        }
    double step = 0.02;
    while (step > 1e-5) {
        bool improved = false;
        const double moves[4][2] = {{step, 0.0}, {-step, 0.0}, {0.0, step}, {0.0, -step}};
        for (const auto& m : moves) {
            const double ll = garch_likelihood(r, n, target, best_a + m[0], best_b + m[1]);
            if (ll > best) best = ll, best_a += m[0], best_b += m[1], improved = true;
        }
        // moving along the ridge alpha + beta = constant often helps
        for (double s : {step, -step}) {
            const double ll = garch_likelihood(r, n, target, best_a + s, best_b - s);
            if (ll > best) best = ll, best_a += s, best_b -= s, improved = true;
        }
        if (!improved) step *= 0.5;
    }
    Garch g;
    g.alpha = best_a;
    g.beta = best_b;
    g.omega = target * (1.0 - best_a - best_b);
    g.log_likelihood = best - 0.5 * static_cast<double>(n) * std::log(6.283185307179586);
    return g;
}

Vector garch_variance_path(const Garch& model, const double* r, std::size_t n) {
    Vector v(n + 1);
    const double persistence = model.alpha + model.beta;
    v[0] = persistence < 1.0 ? model.omega / (1.0 - persistence) : second_moment(r, n);
    for (std::size_t t = 0; t < n; ++t) v[t + 1] = model.omega + model.alpha * r[t] * r[t] + model.beta * v[t];
    return v;
}

}  // namespace rc
