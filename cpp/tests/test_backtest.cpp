#include "check.hpp"
#include "riskcore/backtest.hpp"
#include "riskcore/rng.hpp"

using namespace rc;

void test_backtest(Suite& suite) {
    // exactly the expected number of exceedances: nothing to reject
    CHECK_NEAR(kupiec(1000, 10, 0.01).statistic, 0.0, 1e-12);
    CHECK_NEAR(kupiec(1000, 10, 0.01).p_value, 1.0, 1e-12);
    // 25 against 10 expected: LR = 2 [25 ln(0.025/0.01) + 975 ln(0.975/0.99)]
    const double lr = 2.0 * (25.0 * std::log(2.5) + 975.0 * std::log(0.975 / 0.99));
    CHECK_NEAR(kupiec(1000, 25, 0.01).statistic, lr, 1e-10);
    CHECK(kupiec(1000, 25, 0.01).p_value < 1e-3);
    CHECK(kupiec(1000, 0, 0.01).p_value < 0.01 && kupiec(250, 0, 0.01).p_value > 0.01);

    // independent hits pass, hits in runs fail
    Rng rng(6);
    std::vector<int> iid(5000), runs(5000, 0);
    for (int& h : iid) h = rng.uniform() < 0.05;
    for (std::size_t t = 0; t < runs.size(); t += 100)
        for (std::size_t k = 0; k < 5; ++k) runs[t + k] = 1;
    CHECK(christoffersen_independence(iid).p_value > 0.01);
    CHECK(christoffersen_independence(runs).p_value < 1e-6);
    CHECK(conditional_coverage(iid, 0.05).p_value > 0.01);
    // the clustered series has exactly the right frequency, so only the joint test catches it
    CHECK(kupiec(5000, 250, 0.05).p_value > 0.99 && conditional_coverage(runs, 0.05).p_value < 1e-6);
    CHECK(christoffersen_independence(std::vector<int>(100, 0)).p_value == 1.0);

    const Vector losses = {0.01, 0.05, -0.02, 0.03, 0.00};
    const Vector var = {0.02, 0.02, 0.02, 0.02, 0.02}, es = {0.04, 0.04, 0.04, 0.04, 0.04};
    const std::vector<int> hits = exceedance_hits(losses, var);
    CHECK(hits == (std::vector<int>{0, 1, 0, 1, 0}));
    const ShortfallCheck c = shortfall_check(losses, var, es, 0.4);
    CHECK(c.exceedances == 2);
    CHECK_NEAR(c.severity, 1.0, 1e-14);
    CHECK_NEAR(c.ratio, 1.0, 1e-14);
}
