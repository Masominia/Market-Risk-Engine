#include "check.hpp"
#include "riskcore/rng.hpp"
#include "riskcore/rolling.hpp"

using namespace rc;

void test_rolling(Suite& suite) {
    Rng rng(17);
    // independent normal returns: every method should be close to its nominal level
    Vector r(3250);
    for (double& v : r) v = 0.01 * rng.normal();
    BacktestConfig cfg;
    const BacktestResult res = rolling_backtest(r, cfg);
    CHECK(res.losses.size() == 3000 && res.methods.size() == 7);
    CHECK(res.losses[0] == -r[250]);
    for (const MethodResult& m : res.methods) {
        CHECK(m.exceedances > 12 && m.exceedances < 60);  // 30 expected
        CHECK(m.shortfall.severity > 0.9 && m.shortfall.severity < 1.2);
        CHECK(m.var_series.size() == 3000 && m.mean_var > 0.018 && m.mean_var < 0.028);
    }

    // volatility in long regimes: the unconditional methods cluster their exceedances, the filtered one does not
    Vector regimes(4250);
    for (std::size_t t = 0; t < regimes.size(); ++t) regimes[t] = ((t / 250) % 2 == 0 ? 0.006 : 0.02) * rng.normal();
    const BacktestResult sw = rolling_backtest(regimes, cfg);
    CHECK(sw.methods[0].name == "historical" && sw.methods[4].name == "ewma_filtered");
    CHECK(sw.methods[0].independence.p_value < 0.01);
    CHECK(sw.methods[4].independence.p_value > sw.methods[0].independence.p_value);
    CHECK(sw.methods[4].coverage.p_value > 0.01);

    // minimum variance on two blocks of assets with different volatility
    Matrix panel(800, 6);
    for (std::size_t t = 0; t < panel.rows; ++t)
        for (std::size_t j = 0; j < 6; ++j) panel(t, j) = (j < 3 ? 0.005 : 0.02) * rng.normal();
    const MinVarResult equal = rolling_min_variance(panel, 250, 21, 0), inverse = rolling_min_variance(panel, 250, 21, 1);
    const MinVarResult sample = rolling_min_variance(panel, 250, 21, 2), shrunk = rolling_min_variance(panel, 250, 21, 3);
    CHECK(inverse.volatility < 0.6 * equal.volatility && sample.volatility < 0.6 * equal.volatility && shrunk.volatility < 0.6 * equal.volatility);
    CHECK(equal.turnover == 0.0 && sample.turnover > 0.0 && sample.failures == 0);
    CHECK(shrunk.intensity > 0.0 && shrunk.intensity < 1.0);
    CHECK_NEAR(equal.gross, 1.0, 1e-12);
}
