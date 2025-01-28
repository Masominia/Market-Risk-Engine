# Validation checks

| area | check | status | detail |
|---|---|---|---|
| backtest | filtered historical simulation is closest to the expected exceedance frequency | PASS | 1.31 and 1.35 times the expected number, others 1.38 to 1.98 |
| backtest | the best method is within 20% of the expected frequency | WARN | 1.31 times the expected number: every method still under-forecasts on this sample |
| backtest | the normal method has too many exceedances | PASS | 1.98 times the expected number |
| backtest | filtering reduces rejections of the frequency test | PASS | 1 of 40 portfolios at most, against 36 (normal) and 18 (historical) |
| backtest | filtered expected shortfall is the best calibrated | PASS | shortfall ratio 1.16 and 1.23, others 1.24 to 1.68 |
| backtest | false-alarm rate of the tests is plausible for a good method | PASS | 1 of 40 rejected at the 5% level |
| covariance | shrinkage never increases realised volatility | PASS | reduction -0.2%, 2.6%, 11.3%, 18.6%, 33.9%, 68.8% |
| covariance | the gain grows as stocks approach the number of days | PASS | -0.2% with 25 stocks, 68.8% with 240 |
| covariance | shrunk minimum variance beats the simple portfolios | PASS | largest universe: 15.8% against 22.6% (inverse variance) and 25.8% (equal weight) |
| covariance | no matrix failed to factorise | PASS | 0 failures |
| decomposition | contributions add up to the portfolio volatility | PASS | relative gap 0.0e+00 |
