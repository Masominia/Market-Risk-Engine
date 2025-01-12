#include "check.hpp"
#include "riskcore/covariance.hpp"
#include "riskcore/linalg.hpp"
#include "riskcore/rng.hpp"

using namespace rc;

void test_covariance(Suite& suite) {
    Matrix r(4, 2);
    const double data[4][2] = {{1, 2}, {2, 1}, {3, 5}, {6, 4}};
    for (std::size_t i = 0; i < 4; ++i)
        for (std::size_t j = 0; j < 2; ++j) r(i, j) = data[i][j];
    const Matrix s = sample_covariance(r, 0, 4);
    CHECK_NEAR(s(0, 0), 14.0 / 3.0, 1e-14);
    CHECK_NEAR(s(1, 1), 10.0 / 3.0, 1e-14);
    CHECK_NEAR(s(0, 1), 7.0 / 3.0, 1e-14);
    CHECK(s(0, 1) == s(1, 0));
    const Matrix c = correlation(s);
    CHECK_NEAR(c(0, 0), 1.0, 1e-15);
    CHECK_NEAR(c(0, 1), (7.0 / 3.0) / std::sqrt(140.0 / 9.0), 1e-14);
    CHECK(diagonal_part(s)(0, 1) == 0.0 && diagonal_part(s)(1, 1) == s(1, 1));

    // with lambda = 1 the weighted estimator is the plain second moment
    const Matrix e = ewma_covariance(r, 0, 4, 1.0);
    CHECK_NEAR(e(0, 0), (1.0 + 4.0 + 9.0 + 36.0) / 4.0, 1e-14);
    // with a small lambda only the newest row counts
    CHECK_NEAR(ewma_covariance(r, 0, 4, 1e-9)(0, 1), 24.0, 1e-6);

    // more assets than observations: the sample matrix is singular, the shrunk one is not
    Rng rng(8);
    Matrix wide(40, 60);
    for (double& v : wide.a) v = 0.01 * rng.normal();
    Matrix l;
    CHECK(!cholesky(sample_covariance(wide, 0, 40), l));
    const Shrunk lw = ledoit_wolf(wide, 0, 40);
    CHECK(cholesky(lw.covariance, l));
    CHECK(lw.intensity > 0.5 && lw.intensity <= 1.0);  // the truth is a multiple of the identity
    CHECK_NEAR(trace(lw.covariance), trace(sample_covariance(wide, 0, 40)) * 39.0 / 40.0, 1e-12);

    // many observations of strongly correlated assets: little shrinkage
    Matrix tall(4000, 5);
    for (std::size_t t = 0; t < tall.rows; ++t) {
        const double common = rng.normal();
        for (std::size_t j = 0; j < 5; ++j) tall(t, j) = common * (1.0 + j) + 0.3 * rng.normal();
    }
    CHECK(ledoit_wolf(tall, 0, 4000).intensity < 0.02);
}
