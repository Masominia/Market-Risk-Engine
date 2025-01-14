#include "riskcore/rolling.hpp"

#include <cmath>
#include <stdexcept>

#include "riskcore/covariance.hpp"
#include "riskcore/portfolio.hpp"
#include "riskcore/returns.hpp"
#include "riskcore/risk.hpp"
#include "riskcore/stats.hpp"

namespace rc {

BacktestResult rolling_backtest(const Vector& returns, const BacktestConfig& cfg) {
    if (returns.size() <= cfg.window + 1) throw std::invalid_argument("rolling_backtest: series shorter than the window");
    const auto& names = method_names();
    const std::size_t days = returns.size() - cfg.window;
    BacktestResult out;
    out.losses.resize(days);
    out.methods.resize(names.size());
    std::vector<Vector> var_es_level(names.size(), Vector(days));
    for (std::size_t m = 0; m < names.size(); ++m) {
        out.methods[m].name = names[m];
        out.methods[m].var_series.resize(days);
        out.methods[m].es_series.resize(days);
    }
    Garch garch;
    for (std::size_t d = 0; d < days; ++d) {
        const double* window = returns.data() + d;
        if (d % cfg.garch_refit == 0) garch = fit_garch(window, cfg.window);
        out.losses[d] = -returns[d + cfg.window];
        for (std::size_t m = 0; m < names.size(); ++m) {
            out.methods[m].var_series[d] = estimate(m, window, cfg.window, cfg.var_level, cfg.lambda, garch).var;
            const RiskEstimate tail = estimate(m, window, cfg.window, cfg.es_level, cfg.lambda, garch);
            var_es_level[m][d] = tail.var;
            out.methods[m].es_series[d] = tail.es;
        }
    }
    for (std::size_t m = 0; m < names.size(); ++m) {
        MethodResult& r = out.methods[m];
        const std::vector<int> hits = exceedance_hits(out.losses, r.var_series);
        for (int h : hits) r.exceedances += h != 0;
        r.kupiec = kupiec(days, r.exceedances, 1.0 - cfg.var_level);
        r.independence = christoffersen_independence(hits);
        r.coverage = conditional_coverage(hits, 1.0 - cfg.var_level);
        r.shortfall = shortfall_check(out.losses, var_es_level[m], r.es_series, 1.0 - cfg.es_level);
        r.mean_var = mean(r.var_series);
    }
    return out;
}

const std::vector<std::string>& estimator_names() {
    static const std::vector<std::string> names = {"equal_weight", "inverse_variance", "sample", "ledoit_wolf"};
    return names;
}

MinVarResult rolling_min_variance(const Matrix& returns, std::size_t window, std::size_t rebalance, std::size_t estimator) {
    if (estimator >= estimator_names().size()) throw std::invalid_argument("rolling_min_variance: unknown estimator");
    if (returns.rows <= window + 1) throw std::invalid_argument("rolling_min_variance: panel shorter than the window");
    const std::size_t n = returns.cols;
    MinVarResult out;
    out.name = estimator_names()[estimator];
    Vector weights = equal_weights(n), realised;
    std::size_t rebalances = 0;
    for (std::size_t t = window; t < returns.rows; ++t) {
        if ((t - window) % rebalance == 0) {
            Vector fresh = equal_weights(n);
            bool ok = true;
            if (estimator == 1) {
                const Matrix s = sample_covariance(returns, t - window, t);
                double total = 0.0;
                for (std::size_t i = 0; i < n; ++i) total += fresh[i] = 1.0 / s(i, i);
                for (double& w : fresh) w /= total;
            } else if (estimator == 2) {
                ok = min_variance_weights(sample_covariance(returns, t - window, t), fresh);
            } else if (estimator == 3) {
                const Shrunk s = ledoit_wolf(returns, t - window, t);
                out.intensity += s.intensity;
                ok = min_variance_weights(s.covariance, fresh);
            }
            if (!ok) {
                ++out.failures;
                fresh = weights;  // keep the previous portfolio
            }
            if (rebalances > 0) out.turnover += turnover(weights, fresh);
            weights = fresh;
            out.gross += gross_exposure(weights);
            ++rebalances;
        }
        double r = 0.0;
        const double* row = returns.row(t);
        for (std::size_t i = 0; i < n; ++i) r += weights[i] * row[i];
        realised.push_back(r);
    }
    out.volatility = std_dev(realised) * std::sqrt(252.0);
    out.gross /= static_cast<double>(rebalances);
    out.intensity /= static_cast<double>(rebalances);
    out.turnover /= static_cast<double>(rebalances > 1 ? rebalances - 1 : 1);
    return out;
}

}  // namespace rc
