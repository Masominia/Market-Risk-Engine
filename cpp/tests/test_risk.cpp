#include <algorithm>

#include "check.hpp"
#include "riskcore/normal.hpp"
#include "riskcore/risk.hpp"
#include "riskcore/rng.hpp"

using namespace rc;

void test_risk(Suite& suite) {
    const RiskEstimate n99 = normal_risk_from_moments(0.0, 1.0, 0.99);
    CHECK_NEAR(n99.var, 2.326347874040841, 1e-10);
    CHECK_NEAR(n99.es, 2.665214220345808, 1e-9);
    CHECK_NEAR(normal_risk_from_moments(0.001, 0.02, 0.975).es, 0.02 * 2.337802792380329 - 0.001, 1e-9);

    // 101 equally spaced returns from -0.50 to +0.50: quantiles are exact
    Vector grid(101);
    for (int i = 0; i <= 100; ++i) grid[i] = -0.5 + 0.01 * i;
    std::reverse(grid.begin(), grid.end());  // order must not matter
    const RiskEstimate h = historical_risk(grid.data(), grid.size(), 0.95);
    CHECK_NEAR(h.var, 0.45, 1e-12);
    CHECK_NEAR(h.es, 0.475, 1e-12);

    Rng rng(4);
    Vector z(100000);
    for (double& v : z) v = 0.01 * rng.normal();
    const RiskEstimate hist = historical_risk(z.data(), z.size(), 0.99), norm = normal_risk(z.data(), z.size(), 0.99);
    const RiskEstimate cf = cornish_fisher_risk(z.data(), z.size(), 0.99);
    CHECK_NEAR(hist.var, norm.var, 5e-4);
    CHECK_NEAR(hist.es, norm.es, 6e-4);
    CHECK_NEAR(cf.var, norm.var, 3e-4);
    CHECK_NEAR(cf.es, norm.es, 3e-4);
    CHECK(hist.es > hist.var && cf.es >= cf.var);

    // fat tails: the normal method understates the tail, the expansion moves the right way
    Vector fat(100000);
    for (double& v : fat) v = 0.01 * rng.normal() * (rng.uniform() < 0.05 ? 4.0 : 1.0);
    const RiskEstimate fh = historical_risk(fat.data(), fat.size(), 0.999), fn = normal_risk(fat.data(), fat.size(), 0.999);
    CHECK(fh.var > 1.3 * fn.var);
    CHECK(cornish_fisher_risk(fat.data(), fat.size(), 0.999).var > fn.var);

    // a calm history followed by a turbulent month: the conditional methods react, the unconditional ones barely move
    Vector regime(250);
    for (std::size_t t = 0; t < regime.size(); ++t) regime[t] = (t < 230 ? 0.005 : 0.03) * rng.normal();
    const RiskEstimate slow = normal_risk(regime.data(), regime.size(), 0.99);
    const RiskEstimate fast = ewma_normal_risk(regime.data(), regime.size(), 0.99, 0.94);
    const RiskEstimate fhs = filtered_historical_risk(regime.data(), regime.size(), 0.99, ewma_variance_path(regime.data(), regime.size(), 0.94));
    CHECK(fast.var > 1.5 * slow.var && fhs.var > 1.5 * slow.var);

    const Garch g = fit_garch(regime.data(), regime.size());
    for (std::size_t m = 0; m < method_names().size(); ++m) {
        const RiskEstimate e = estimate(m, regime.data(), regime.size(), 0.99, 0.94, g);
        const RiskEstimate lower = estimate(m, regime.data(), regime.size(), 0.95, 0.94, g);
        CHECK(e.var > 0.0 && e.es >= e.var && lower.var < e.var);
    }
    CHECK(method_names().size() == 7);
}
