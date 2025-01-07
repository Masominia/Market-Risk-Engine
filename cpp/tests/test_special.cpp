#include "check.hpp"
#include "riskcore/special.hpp"

using namespace rc;

void test_special(Suite& suite) {
    // P(1, x) = 1 - exp(-x)
    for (double x : {0.1, 1.0, 3.0, 12.0}) CHECK_NEAR(gamma_p(1.0, x), 1.0 - std::exp(-x), 1e-14);
    // P(1/2, x) = erf(sqrt(x))
    for (double x : {0.05, 0.7, 2.0, 9.0}) CHECK_NEAR(gamma_p(0.5, x), std::erf(std::sqrt(x)), 1e-13);
    // textbook critical values of the chi-square distribution
    CHECK_NEAR(chi2_survival(3.841458820694124, 1.0), 0.05, 1e-12);
    CHECK_NEAR(chi2_survival(6.634896601021213, 1.0), 0.01, 1e-12);
    CHECK_NEAR(chi2_survival(5.991464547107979, 2.0), 0.05, 1e-12);
    CHECK_NEAR(chi2_survival(18.307038053275146, 10.0), 0.05, 1e-12);
    CHECK(chi2_survival(0.0, 1.0) == 1.0 && chi2_survival(-1.0, 3.0) == 1.0);
    CHECK(std::isnan(gamma_p(-1.0, 1.0)));
}
