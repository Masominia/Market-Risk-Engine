# Methods

Losses are positive. Value-at-risk at level c is the c-quantile of the loss, expected shortfall is the mean loss beyond it. A window holds the n most recent returns.

## Risk estimators

- **Historical simulation.** Empirical quantile of the window with linear interpolation between order statistics. Expected shortfall is the integral of the interpolated quantile function over the tail, so the two are consistent.
- **Normal.** Sample mean and standard deviation, normal quantile. Expected shortfall is `sigma phi(z) / (1 - c) - mu`.
- **Cornish-Fisher.** The normal quantile corrected with sample skewness and excess kurtosis to third order. Expected shortfall averages the corrected quantile over the tail and is never reported below the value-at-risk, since the expansion is not monotone for strong departures from normality.
- **EWMA, normal.** Variance from `v(t+1) = lambda v(t) + (1 - lambda) r(t)^2`, started at the mean square of the window, zero mean, normal quantile.
- **GARCH, normal.** Variance from `v(t+1) = omega + alpha r(t)^2 + beta v(t)`. The long-run variance is fixed at the mean square of the window, which leaves alpha and beta. They maximise the normal likelihood by a grid followed by a pattern search that also moves along the line of constant alpha + beta.
- **Filtered historical simulation**, with either variance model. Each return of the window is divided by the volatility that was forecast for its day. The empirical quantile and tail mean of these residuals are multiplied by the forecast for the next day.

## Backtests

Let x be the number of days, out of n, on which the loss exceeded the value-at-risk, and p the tail probability.

- **Frequency (Kupiec).** Likelihood ratio of the binomial with probability p against the binomial with probability x/n, compared with a chi-square distribution with one degree of freedom.
- **Clustering (Christoffersen).** The exceedance indicator is modelled as a two-state Markov chain. The likelihood ratio compares equal transition probabilities with free ones, one degree of freedom. If no exceedance is ever followed by another day, the test returns no evidence.
- **Joint.** The sum of the two statistics, two degrees of freedom.
- **Expected shortfall.** Two ratios, both equal to one for a correct model. The shortfall ratio is the sum of loss over expected shortfall on exceedance days, divided by n p. Severity is the mean of loss over expected shortfall on exceedance days only. No p-value is attached to either.

The chi-square distribution comes from the regularised incomplete gamma function, computed by its series below a + 1 and by a continued fraction above.

## Covariance

- **Sample.** Centred, divisor n - 1.
- **Exponentially weighted.** Zero mean, weights lambda^k from the newest row backwards.
- **Ledoit-Wolf.** `(1 - d) S + d m I`, with S the centred second-moment matrix (divisor n), m the mean of its diagonal, and d the ratio of the estimated sampling error of S to its squared distance from `m I`, clipped to [0, 1]. The sampling error uses the identity `sum_k |x_k x_k' - S|^2 = sum_k (x_k' x_k)^2 - n |S|^2`, which avoids forming one matrix per observation.

## Portfolios

Minimum-variance weights are `C^-1 1 / (1' C^-1 1)` through a Cholesky factorisation. If the factorisation fails the previous weights are kept and the failure is counted. The Euler decomposition gives asset i the contribution `w_i (C w)_i / sigma`, and the contributions sum to sigma.

## Rolling drivers

`rolling_backtest` moves the window one day at a time. All methods see the same window and the GARCH parameters are refitted at a fixed interval. `rolling_min_variance` recomputes the weights at a fixed interval and holds them in between, with daily rebalancing to those weights.
