#include <cstdio>
#include <fstream>

#include "check.hpp"
#include "riskcore/csv.hpp"
#include "riskcore/returns.hpp"

using namespace rc;

void test_csv(Suite& suite) {
    const std::string path = "riskcore_test_prices.csv";
    {
        std::ofstream f(path);
        f << "day,AAA,BBB\n0,100,50\n1,110,45\n2,99,54\n";
    }
    const Panel p = read_prices(path);
    CHECK(p.names.size() == 2 && p.names[1] == "BBB");
    CHECK(p.prices.rows == 3 && p.prices.cols == 2 && p.prices(2, 1) == 54.0);
    const Matrix r = simple_returns(p.prices);
    CHECK(r.rows == 2);
    CHECK_NEAR(r(0, 0), 0.10, 1e-15);
    CHECK_NEAR(r(1, 1), 0.20, 1e-15);
    const Vector pr = portfolio_returns(r, equal_weights(2));
    CHECK_NEAR(pr[0], 0.0, 1e-15);
    CHECK_NEAR(pr[1], 0.05, 1e-15);
    CHECK(simple_returns(p.prices, 1).cols == 1);

    {
        std::ofstream f(path);
        f << "day,AAA,BBB\n0,100\n";
    }
    bool threw = false;
    try {
        read_prices(path);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
    std::remove(path.c_str());
    threw = false;
    try {
        read_prices("no_such_file.csv");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
}
