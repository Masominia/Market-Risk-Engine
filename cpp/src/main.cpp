// Command line front end: riskcore <command> <prices.csv> key=value ...
// Output: `key value` lines and `row ...` lines, read by the Python package.
//
//   describe   size of the panel and moments of the equal-weight portfolio
//   backtest   rolling value-at-risk and expected shortfall backtest of several portfolios
//   minvar     out-of-sample volatility of minimum-variance portfolios by covariance estimator
//   decompose  risk contributions of the equal-weight portfolio

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>

#include "riskcore/covariance.hpp"
#include "riskcore/csv.hpp"
#include "riskcore/linalg.hpp"
#include "riskcore/portfolio.hpp"
#include "riskcore/returns.hpp"
#include "riskcore/rng.hpp"
#include "riskcore/rolling.hpp"
#include "riskcore/stats.hpp"

using namespace rc;

struct Args {
    std::map<std::string, std::string> kv;
    double num(const std::string& key, double fallback) const {
        const auto it = kv.find(key);
        return it == kv.end() ? fallback : std::stod(it->second);
    }
    std::size_t count(const std::string& key, std::size_t fallback) const { return static_cast<std::size_t>(num(key, static_cast<double>(fallback))); }
};

// Portfolio 0 holds every asset equally; the others hold `holdings` random assets with random positive weights.
static Vector make_weights(std::size_t index, std::size_t assets, std::size_t holdings, Rng& rng) {
    if (index == 0) return equal_weights(assets);
    Vector w(assets, 0.0);
    double total = 0.0;
    for (std::size_t k = 0; k < holdings; ++k) {
        const std::size_t j = rng.below(assets);
        const double x = -std::log(rng.uniform());
        w[j] += x;
        total += x;
    }
    for (double& x : w) x /= total;
    return w;
}

static int run(const std::string& command, const std::string& path, const Args& a) {
    const Panel panel = read_prices(path);
    const std::size_t assets = std::min(a.count("assets", panel.names.size()), panel.names.size());
    const Matrix returns = simple_returns(panel.prices, assets);

    if (command == "describe") {
        const Vector r = portfolio_returns(returns, equal_weights(assets));
        std::printf("days %zu\nassets %zu\n", returns.rows, assets);
        std::printf("volatility %.10g\nskewness %.10g\nexcess_kurtosis %.10g\n", std_dev(r) * std::sqrt(252.0), skewness(r.data(), r.size()), excess_kurtosis(r.data(), r.size()));
        std::printf("worst_day %.10g\nbest_day %.10g\n", *std::min_element(r.begin(), r.end()), *std::max_element(r.begin(), r.end()));
        return 0;
    }

    if (command == "backtest") {
        BacktestConfig cfg;
        cfg.window = a.count("window", cfg.window);
        cfg.var_level = a.num("var_level", cfg.var_level);
        cfg.es_level = a.num("es_level", cfg.es_level);
        cfg.lambda = a.num("lambda", cfg.lambda);
        cfg.garch_refit = a.count("garch_refit", cfg.garch_refit);
        const std::size_t portfolios = a.count("portfolios", 1), holdings = a.count("holdings", 20);
        Rng rng(static_cast<std::uint64_t>(a.num("seed", 1)));
        std::printf("out_of_sample_days %zu\n", returns.rows - cfg.window);
        for (std::size_t k = 0; k < portfolios; ++k) {
            const Vector r = portfolio_returns(returns, make_weights(k, assets, holdings, rng));
            const BacktestResult res = rolling_backtest(r, cfg);
            for (const MethodResult& m : res.methods)
                std::printf("row %zu %s %zu %.10g %.10g %.10g %zu %.10g %.10g %.10g\n", k, m.name.c_str(), m.exceedances, m.kupiec.p_value, m.independence.p_value,
                            m.coverage.p_value, m.shortfall.exceedances, m.shortfall.ratio, m.shortfall.severity, m.mean_var);
            const auto it = a.kv.find("out");
            if (k == 0 && it != a.kv.end()) {
                std::ofstream f(it->second);
                f << "day,loss";
                for (const MethodResult& m : res.methods) f << ',' << m.name;
                f << '\n';
                for (std::size_t d = 0; d < res.losses.size(); ++d) {
                    f << d << ',' << res.losses[d];
                    for (const MethodResult& m : res.methods) f << ',' << m.var_series[d];
                    f << '\n';
                }
            }
        }
        return 0;
    }

    if (command == "minvar") {
        const std::size_t window = a.count("window", 250), rebalance = a.count("rebalance", 21);
        const Matrix first = sample_covariance(returns, 0, window);
        std::printf("assets %zu\nwindow %zu\n", assets, window);
        std::printf("condition_sample %.6g\ncondition_shrunk %.6g\n", condition_number(first), condition_number(ledoit_wolf(returns, 0, window).covariance));
        for (std::size_t e = 0; e < estimator_names().size(); ++e) {
            const MinVarResult r = rolling_min_variance(returns, window, rebalance, e);
            std::printf("row %s %.10g %.10g %.10g %.10g %zu\n", r.name.c_str(), r.volatility, r.gross, r.turnover, r.intensity, r.failures);
        }
        return 0;
    }

    if (command == "decompose") {
        const std::size_t window = std::min(a.count("window", 250), returns.rows);
        const Matrix cov = ledoit_wolf(returns, returns.rows - window, returns.rows).covariance;
        const Vector w = equal_weights(assets);
        const Vector c = risk_contributions(w, cov);
        const double vol = std::sqrt(portfolio_variance(w, cov));
        std::printf("volatility %.12g\nsum_of_contributions %.12g\n", vol, std::accumulate(c.begin(), c.end(), 0.0));
        std::vector<std::size_t> order(assets);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](std::size_t i, std::size_t j) { return c[i] > c[j]; });
        for (std::size_t k = 0; k < std::min<std::size_t>(a.count("top", 10), assets); ++k)
            std::printf("row %s %.10g %.10g\n", panel.names[order[k]].c_str(), c[order[k]] / vol, std::sqrt(cov(order[k], order[k]) * 252.0));
        return 0;
    }

    throw std::invalid_argument("unknown command " + command);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: riskcore <describe|backtest|minvar|decompose> <prices.csv> key=value ...\n");
        return 1;
    }
    Args a;
    for (int i = 3; i < argc; ++i) {
        const std::string s = argv[i];
        const auto eq = s.find('=');
        if (eq != std::string::npos) a.kv[s.substr(0, eq)] = s.substr(eq + 1);
    }
    try {
        return run(argv[1], argv[2], a);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "riskcore: %s\n", e.what());
        return 1;
    }
}
