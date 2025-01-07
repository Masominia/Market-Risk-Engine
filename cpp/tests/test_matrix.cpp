#include "check.hpp"
#include "riskcore/matrix.hpp"

using namespace rc;

void test_matrix(Suite& suite) {
    Matrix a(2, 3);
    double v = 1.0;
    for (std::size_t i = 0; i < 2; ++i)
        for (std::size_t j = 0; j < 3; ++j) a(i, j) = v++;
    const Matrix t = transpose(a);
    CHECK(t.rows == 3 && t.cols == 2 && t(2, 0) == 3.0 && t(0, 1) == 4.0);
    const Matrix g = multiply(a, t);
    CHECK(g.rows == 2 && g(0, 0) == 14.0 && g(0, 1) == 32.0 && g(1, 1) == 77.0);
    const Vector y = multiply(a, Vector{1.0, 0.0, -1.0});
    CHECK(y[0] == -2.0 && y[1] == -2.0);
    CHECK(dot(Vector{1, 2, 3}, Vector{4, 5, 6}) == 32.0);
    CHECK(trace(identity(4)) == 4.0 && frobenius_squared(a) == 91.0);
    bool threw = false;
    try {
        multiply(a, a);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}
