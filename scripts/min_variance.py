"""Out-of-sample volatility of minimum-variance portfolios on the bundled panel.

python scripts/min_variance.py [stocks] [window] [rebalance]
"""

import sys

from riskcore import engine

if __name__ == "__main__":
    assets, window, rebalance = (int(x) for x in (sys.argv[1:4] if len(sys.argv) >= 4 else (100, 250, 21)))
    res = engine.call("minvar", assets=assets, window=window, rebalance=rebalance)
    print(f"condition number: sample {res['condition_sample']:.0f}, shrunk {res['condition_shrunk']:.0f}")
    for name, vol, gross, turnover, _intensity, failures in res["rows"]:
        print(
            f"{name:<18} volatility {vol:6.1%}  gross {gross:5.2f}  turnover {turnover:5.2f}  failures {int(failures)}"
        )
