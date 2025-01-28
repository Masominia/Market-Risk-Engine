# RiskCore report

Data: 300 stocks, 1361 daily returns. The equal-weight portfolio has an annualised volatility of 24.4%, skewness -0.57, excess kurtosis 10.2, worst day -12.9%.

## 1. Backtest of value-at-risk and expected shortfall

Every day each method sees the previous 250 returns and forecasts the next day: value-at-risk at 99%, expected shortfall at 97.5%. 1,111 out-of-sample days, 40 portfolios (all stocks equally weighted, then 39 random portfolios of 20 stocks).

All portfolios together. Rejections are counts of portfolios, out of 40, at the 5% level.

| method | exceedances | expected | ratio | frequency rejected | clustering rejected | joint test rejected | shortfall ratio | severity | mean value-at-risk |
|---|---|---|---|---|---|---|---|---|---|
| historical simulation | 720 | 444.400 | 1.620 | 18 | 21 | 24 | 1.463 | 1.165 | 0.047 |
| normal | 881 | 444.400 | 1.982 | 36 | 16 | 37 | 1.681 | 1.308 | 0.040 |
| Cornish-Fisher | 615 | 444.400 | 1.384 | 5 | 10 | 11 | 1.238 | 1.074 | 0.053 |
| EWMA, normal | 851 | 444.400 | 1.915 | 34 | 2 | 30 | 1.487 | 1.124 | 0.038 |
| EWMA, filtered historical | 583 | 444.400 | 1.312 | 0 | 0 | 0 | 1.163 | 1.051 | 0.045 |
| GARCH, normal | 746 | 444.400 | 1.679 | 24 | 3 | 18 | 1.385 | 1.124 | 0.038 |
| GARCH, filtered historical | 599 | 444.400 | 1.348 | 1 | 2 | 2 | 1.233 | 1.070 | 0.041 |

Shortfall ratio: sum of loss / expected shortfall on exceedance days over the expected number of exceedances. Severity: mean of loss / expected shortfall on exceedance days. Both are 1 for a correct model.

The equal-weight portfolio alone:

| method | exceedances | frequency p-value | clustering p-value | joint p-value | shortfall ratio | severity | mean value-at-risk |
|---|---|---|---|---|---|---|---|
| historical simulation | 19 | 0.031 | 0.040 | 0.012 | 1.530 | 1.181 | 0.045 |
| normal | 27 | 5.12e-05 | 0.167 | 1.06e-04 | 1.808 | 1.395 | 0.035 |
| Cornish-Fisher | 17 | 0.099 | 0.258 | 0.136 | 1.356 | 1.076 | 0.048 |
| EWMA, normal | 24 | 7.58e-04 | 0.543 | 2.86e-03 | 1.568 | 1.146 | 0.033 |
| EWMA, filtered historical | 13 | 0.579 | 0.579 | 0.735 | 1.041 | 1.033 | 0.040 |
| GARCH, normal | 22 | 3.78e-03 | 0.453 | 0.011 | 1.333 | 1.157 | 0.033 |
| GARCH, filtered historical | 13 | 0.579 | 0.579 | 0.735 | 1.064 | 1.019 | 0.038 |

## 2. Minimum-variance portfolios

Weights from the previous 250 days, recomputed every 21 days, short positions allowed. Annualised out-of-sample volatility:

| stocks | equal weight | inverse variance | minimum variance, sample covariance | minimum variance, shrunk covariance |
|---|---|---|---|---|
| 25 | 0.2900 | 0.2477 | 0.2192 | 0.2195 |
| 50 | 0.2593 | 0.2230 | 0.1895 | 0.1845 |
| 100 | 0.2519 | 0.2174 | 0.1911 | 0.1694 |
| 150 | 0.2524 | 0.2219 | 0.2009 | 0.1636 |
| 200 | 0.2538 | 0.2236 | 0.2395 | 0.1583 |
| 240 | 0.2580 | 0.2259 | 0.5061 | 0.1581 |

Gross exposure (sum of absolute weights):

| stocks | minimum variance, sample covariance | minimum variance, shrunk covariance |
|---|---|---|
| 25 | 1.58 | 1.39 |
| 50 | 2.49 | 1.82 |
| 100 | 3.55 | 2.40 |
| 150 | 5.54 | 2.92 |
| 200 | 8.86 | 3.36 |
| 240 | 20.86 | 3.43 |

Turnover per rebalance:

| stocks | minimum variance, sample covariance | minimum variance, shrunk covariance |
|---|---|---|
| 25 | 0.19 | 0.13 |
| 50 | 0.41 | 0.22 |
| 100 | 0.78 | 0.35 |
| 150 | 1.47 | 0.48 |
| 200 | 3.12 | 0.61 |
| 240 | 12.47 | 0.66 |

Condition number of the first covariance matrix:

| stocks | sample covariance | shrunk covariance |
|---|---|---|
| 25 | 78 | 25 |
| 50 | 2,362 | 64 |
| 100 | 5,776 | 119 |
| 150 | 12,692 | 169 |
| 200 | 39,491 | 250 |
| 240 | 306,229 | 268 |

Mean shrinkage intensity: 0.088 to 0.097.

## 3. Risk decomposition of the equal-weight portfolio

Daily volatility 1.3862% (shrunk covariance of the last 250 days). Largest contributors, each holding 1/300 of the capital:

| stock | share of portfolio risk | own volatility |
|---|---|---|
| SFIX | 9.15e-03 | 0.9747 |
| RIOT | 8.60e-03 | 1.0407 |
| BLFS | 7.88e-03 | 0.8666 |
| RUN | 7.70e-03 | 0.8412 |
| AKTS | 7.38e-03 | 0.7484 |
| VCYT | 7.30e-03 | 0.7382 |
| CONN | 7.08e-03 | 0.8207 |
| BAND | 7.00e-03 | 0.9351 |

## Validation

10 PASS, 1 WARN, 0 FAIL. See `validation.md`.
