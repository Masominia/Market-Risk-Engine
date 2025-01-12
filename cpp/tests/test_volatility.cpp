#include "check.hpp"
#include "riskcore/rng.hpp"
#include "riskcore/volatility.hpp"

using namespace rc;

void test_volatility(Suite& suite) {
    const Vector r = {0.01, -0.02, 0.03};
    const Vector v = ewma_variance_path(r.data(), r.size(), 0.9);
    const double v0 = (1e-4 + 4e-4 + 9e-4) / 3.0;
    CHECK(v.size() == 4);
    CHECK_NEAR(v[0], v0, 1e-18);
    CHECK_NEAR(v[1], 0.9 * v0 + 0.1 * 1e-4, 1e-18);
    CHECK_NEAR(v[3], 0.9 * (0.9 * v[1] + 0.1 * 4e-4) + 0.1 * 9e-4, 1e-18);

    // simulate a GARCH(1,1) process and recover its parameters
    Rng rng(21);
    const double omega = 2e-6, alpha = 0.08, beta = 0.90;
    Vector x(20000);
    double var = omega / (1.0 - alpha - beta);
    for (double& xt : x) {
        xt = std::sqrt(var) * rng.normal();
        var = omega + alpha * xt * xt + beta * var;
    }
    const Garch g = fit_garch(x.data(), x.size());
    CHECK_NEAR(g.alpha, alpha, 0.015);
    CHECK_NEAR(g.beta, beta, 0.02);
    CHECK(g.omega > 0.0 && g.alpha + g.beta < 1.0);
    const Vector path = garch_variance_path(g, x.data(), x.size());
    CHECK(path.size() == x.size() + 1);
    CHECK_NEAR(path[1], g.omega + g.alpha * x[0] * x[0] + g.beta * path[0], 1e-18);

    // on constant-variance noise the fit must not invent persistence in the squares
    Vector flat(5000);
    for (double& f : flat) f = 0.01 * rng.normal();
    CHECK(fit_garch(flat.data(), flat.size()).alpha < 0.03);
}
