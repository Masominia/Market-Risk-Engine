#include "check.hpp"
#include "riskcore/rng.hpp"
#include "riskcore/stats.hpp"

using namespace rc;

void test_stats(Suite& suite) {
    const Vector x = {2, 4, 4, 4, 5, 5, 7, 9};
    CHECK_NEAR(mean(x), 5.0, 1e-15);
    CHECK_NEAR(variance(x), 32.0 / 7.0, 1e-14);
    const Vector s = {1, 2, 3, 4, 5};
    CHECK_NEAR(quantile_sorted(s, 0.5), 3.0, 1e-15);
    CHECK_NEAR(quantile_sorted(s, 0.1), 1.4, 1e-14);
    CHECK_NEAR(quantile_sorted(s, 1.0), 5.0, 1e-15);
    CHECK_NEAR(quantile(x.data(), x.size(), 0.0), 2.0, 1e-15);
    // tail mean: average of the interpolated quantile function over [0, p]
    CHECK_NEAR(lower_tail_mean_sorted(s, 1.0), 3.0, 1e-14);
    CHECK_NEAR(lower_tail_mean_sorted(s, 0.25), 1.5, 1e-14);
    CHECK(lower_tail_mean_sorted(s, 0.1) < quantile_sorted(s, 0.1));

    Rng rng(5);
    Vector z(200000), e(200000);
    for (std::size_t i = 0; i < z.size(); ++i) {
        z[i] = rng.normal();
        e[i] = -std::log(rng.uniform());
    }
    CHECK_NEAR(mean(z), 0.0, 0.01);
    CHECK_NEAR(std_dev(z), 1.0, 0.01);
    CHECK_NEAR(skewness(z.data(), z.size()), 0.0, 0.03);
    CHECK_NEAR(excess_kurtosis(z.data(), z.size()), 0.0, 0.06);
    CHECK_NEAR(skewness(e.data(), e.size()), 2.0, 0.1);
    CHECK_NEAR(excess_kurtosis(e.data(), e.size()), 6.0, 0.8);
}
