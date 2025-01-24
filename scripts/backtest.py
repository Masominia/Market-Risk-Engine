"""Backtest your own price file (first column a day counter, then one column per asset).

python scripts/backtest.py prices.csv [window=250] [var_level=0.99] [portfolios=1]
"""

import sys

from riskcore import engine

if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    params = dict(a.split("=", 1) for a in sys.argv[2:])
    res = engine.call("backtest", sys.argv[1], **params)
    print(f"{int(res['out_of_sample_days'])} out-of-sample days")
    print(
        f"{'portfolio':>9} {'method':<16} {'exceedances':>11} {'frequency p':>12} {'clustering p':>13} {'severity':>9}"
    )
    for port, method, exceed, kupiec, indep, _cc, _n, _ratio, severity, _var in res["rows"]:
        print(f"{int(port):>9} {method:<16} {int(exceed):>11} {kupiec:>12.3f} {indep:>13.3f} {severity:>9.2f}")
