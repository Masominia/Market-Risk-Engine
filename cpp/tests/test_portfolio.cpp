#include "check.hpp"
#include "riskcore/portfolio.hpp"

using namespace rc;

void test_portfolio(Suite& suite) {
    Matrix c(2, 2);
    c(0, 0) = 0.04;
    c(1, 1) = 0.09;
    c(0, 1) = c(1, 0) = 0.012;
    const Vector w = {0.6, 0.4};
    const double var = 0.36 * 0.04 + 0.16 * 0.09 + 2 * 0.24 * 0.012;
    CHECK_NEAR(portfolio_variance(w, c), var, 1e-15);
    const Vector rc_ = risk_contributions(w, c);
    CHECK_NEAR(rc_[0] + rc_[1], std::sqrt(var), 1e-15);
    CHECK_NEAR(rc_[0], 0.6 * (0.6 * 0.04 + 0.4 * 0.012) / std::sqrt(var), 1e-15);

    Vector mv;
    CHECK(min_variance_weights(c, mv));
    // two assets: w1 = (s2^2 - s12) / (s1^2 + s2^2 - 2 s12)
    CHECK_NEAR(mv[0], (0.09 - 0.012) / (0.04 + 0.09 - 0.024), 1e-14);
    CHECK_NEAR(mv[0] + mv[1], 1.0, 1e-15);
    CHECK(portfolio_variance(mv, c) < portfolio_variance(w, c) && portfolio_variance(mv, c) < 0.04);
    // at the minimum every asset has the same marginal risk
    const Vector m = multiply(c, mv);
    CHECK_NEAR(m[0], m[1], 1e-15);

    Matrix singular(2, 2, 1.0);
    CHECK(!min_variance_weights(singular, mv));
    CHECK_NEAR(gross_exposure(Vector{1.5, -0.5}), 2.0, 1e-15);
    CHECK_NEAR(turnover(Vector{0.5, 0.5}, Vector{0.8, 0.2}), 0.3, 1e-15);
}
