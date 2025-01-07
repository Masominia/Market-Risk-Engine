#include "check.hpp"
#include "riskcore/linalg.hpp"
#include "riskcore/rng.hpp"

using namespace rc;

static Matrix random_spd(std::size_t n, Rng& rng) {
    Matrix b(n + 3, n);
    for (double& v : b.a) v = rng.normal();
    return multiply(transpose(b), b);
}

void test_linalg(Suite& suite) {
    Rng rng(3);
    const std::size_t n = 12;
    const Matrix a = random_spd(n, rng);
    Matrix l;
    CHECK(cholesky(a, l));
    const Matrix back = multiply(l, transpose(l));
    double worst = 0.0;
    for (std::size_t i = 0; i < a.a.size(); ++i) worst = std::max(worst, std::fabs(back.a[i] - a.a[i]));
    CHECK(worst < 1e-10);

    Vector x_true(n);
    for (double& v : x_true) v = rng.normal();
    const Vector x = cholesky_solve(l, multiply(a, x_true));
    worst = 0.0;
    for (std::size_t i = 0; i < n; ++i) worst = std::max(worst, std::fabs(x[i] - x_true[i]));
    CHECK(worst < 1e-8);

    Matrix bad = identity(3);
    bad(2, 2) = -1.0;
    CHECK(!cholesky(bad, l));

    Matrix vecs;
    const Vector e = symmetric_eigenvalues(a, &vecs);
    double tr = 0.0;
    for (double v : e) tr += v;
    CHECK_NEAR(tr, trace(a), 1e-9 * trace(a));
    CHECK(e.front() > 0.0 && std::is_sorted(e.begin(), e.end()));
    // A v = lambda v for the largest eigenpair
    Vector v(n);
    for (std::size_t k = 0; k < n; ++k) v[k] = vecs(k, n - 1);
    const Vector av = multiply(a, v);
    worst = 0.0;
    for (std::size_t k = 0; k < n; ++k) worst = std::max(worst, std::fabs(av[k] - e.back() * v[k]));
    CHECK(worst < 1e-9 * e.back());

    Matrix d = identity(3);
    d(0, 0) = 4.0;
    d(2, 2) = 0.5;
    CHECK_NEAR(condition_number(d), 8.0, 1e-12);
    CHECK(std::isinf(condition_number(bad)));
}
