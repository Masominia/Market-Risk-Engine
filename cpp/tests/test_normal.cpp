#include "check.hpp"
#include "riskcore/normal.hpp"

using namespace rc;

void test_normal(Suite& suite) {
    CHECK_NEAR(normal_cdf(0.0), 0.5, 1e-16);
    CHECK_NEAR(normal_cdf(1.0), 0.8413447460685429, 1e-15);
    CHECK_NEAR(normal_cdf(-2.326347874040841), 0.01, 1e-15);
    CHECK_NEAR(normal_pdf(0.0), 0.3989422804014327, 1e-16);
    CHECK_NEAR(normal_quantile(0.99), 2.326347874040841, 1e-12);
    CHECK_NEAR(normal_quantile(0.975), 1.959963984540054, 1e-12);
    CHECK_NEAR(normal_quantile(0.5), 0.0, 1e-15);
    for (double p : {1e-10, 1e-4, 0.02, 0.3, 0.77, 0.999, 1.0 - 1e-9}) CHECK_NEAR(normal_cdf(normal_quantile(p)) / p, 1.0, 1e-10);
    CHECK(std::isnan(normal_quantile(1.5)) && std::isinf(normal_quantile(0.0)));
}
