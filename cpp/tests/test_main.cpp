#include <cstdio>

#include "check.hpp"

int main() {
    Suite suite;
    struct Entry {
        const char* name;
        void (*run)(Suite&);
    };
    const Entry entries[] = {{"matrix", test_matrix},         {"stats", test_stats},           {"normal", test_normal},   {"special", test_special},
                             {"linalg", test_linalg},         {"covariance", test_covariance}, {"portfolio", test_portfolio}, {"volatility", test_volatility},
                             {"risk", test_risk},             {"backtest", test_backtest},     {"csv", test_csv},         {"rolling", test_rolling}};
    for (const Entry& e : entries) {
        const int before = suite.failures, checks = suite.checks;
        e.run(suite);
        std::printf("%-12s %3d checks  %s\n", e.name, suite.checks - checks, suite.failures == before ? "ok" : "FAILED");
    }
    std::printf("%d checks, %d failures\n", suite.checks, suite.failures);
    return suite.failures == 0 ? 0 : 1;
}
