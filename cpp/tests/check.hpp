// A very small test harness: CHECK records a failure and carries on, each test file registers one function.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

struct Suite {
    int checks = 0;
    int failures = 0;
    void record(bool ok, const char* expr, const char* file, int line) {
        ++checks;
        if (!ok) {
            ++failures;
            std::printf("FAILED %s:%d  %s\n", file, line, expr);
        }
    }
};

#define CHECK(cond) suite.record((cond), #cond, __FILE__, __LINE__)
#define CHECK_NEAR(x, y, tol) suite.record(std::fabs((x) - (y)) <= (tol), #x " ~ " #y, __FILE__, __LINE__)

void test_matrix(Suite& suite);
void test_stats(Suite& suite);
void test_normal(Suite& suite);
void test_special(Suite& suite);
void test_linalg(Suite& suite);
void test_covariance(Suite& suite);
void test_portfolio(Suite& suite);
void test_volatility(Suite& suite);
void test_risk(Suite& suite);
void test_backtest(Suite& suite);
void test_csv(Suite& suite);
void test_rolling(Suite& suite);
