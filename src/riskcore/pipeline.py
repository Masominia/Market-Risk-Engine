"""All experiments, checks and the report."""

from __future__ import annotations

import json
from pathlib import Path

import pandas as pd
import yaml

from . import engine, plotting
from .engine import ROOT
from .report import md_table

LABELS = {
    "historical": "historical simulation",
    "normal": "normal",
    "cornish_fisher": "Cornish-Fisher",
    "ewma_normal": "EWMA, normal",
    "ewma_filtered": "EWMA, filtered historical",
    "garch_normal": "GARCH, normal",
    "garch_filtered": "GARCH, filtered historical",
    "equal_weight": "equal weight",
    "inverse_variance": "inverse variance",
    "sample": "minimum variance, sample covariance",
    "ledoit_wolf": "minimum variance, shrunk covariance",
}
COLS = [
    "portfolio",
    "method",
    "exceedances",
    "kupiec",
    "independence",
    "coverage",
    "es_exceedances",
    "es_ratio",
    "severity",
    "mean_var",
]


def _check(checks, area, name, ok, detail, warn=False):
    checks.append(
        {"area": area, "check": name, "status": "PASS" if ok else ("WARN" if warn else "FAIL"), "detail": detail}
    )


def run(cfg_path=None, out=None, fast: bool = False) -> dict:
    cfg = yaml.safe_load(open(cfg_path or ROOT / "configs" / "default.yaml"))
    out = Path(out) if out else ROOT / "reports"
    (out / "figures").mkdir(parents=True, exist_ok=True)
    if not engine.available():
        raise SystemExit("C++ engine not built: run `make build` first")
    checks: list[dict] = []
    n_port = 8 if fast else cfg["portfolios"]
    sizes = cfg["minvar_assets"][:3] if fast else cfg["minvar_assets"]
    p_var = 1 - cfg["var_level"]

    desc = engine.call("describe")

    # 1. value-at-risk and expected shortfall backtest
    series = out / "var_series.csv"
    bt = engine.call(
        "backtest",
        window=cfg["window"],
        var_level=cfg["var_level"],
        es_level=cfg["es_level"],
        **{"lambda": cfg["lambda"]},
        garch_refit=cfg["garch_refit"],
        portfolios=n_port,
        holdings=cfg["holdings"],
        seed=cfg["seed"],
        out=series,
    )
    days = int(bt["out_of_sample_days"])
    rows = pd.DataFrame(bt["rows"], columns=COLS)
    rows["method"] = rows["method"].map(LABELS)
    order = [LABELS[m] for m in list(LABELS)[:7]]
    g = rows.groupby("method")
    agg = pd.DataFrame(
        {
            "exceedances": g["exceedances"].sum().astype(int),
            "expected": n_port * days * p_var,
            "ratio": g["exceedances"].sum() / (n_port * days * p_var),
            "frequency rejected": g["kupiec"].apply(lambda s: int((s < 0.05).sum())),
            "clustering rejected": g["independence"].apply(lambda s: int((s < 0.05).sum())),
            "joint test rejected": g["coverage"].apply(lambda s: int((s < 0.05).sum())),
            "shortfall ratio": g["es_ratio"].mean(),
            "severity": g["severity"].mean(),
            "mean value-at-risk": g["mean_var"].mean(),
        }
    ).loc[order]
    eq = (
        rows[rows["portfolio"] == 0]
        .set_index("method")
        .loc[order, ["exceedances", "kupiec", "independence", "coverage", "es_ratio", "severity", "mean_var"]]
    )
    eq.columns = [
        "exceedances",
        "frequency p-value",
        "clustering p-value",
        "joint p-value",
        "shortfall ratio",
        "severity",
        "mean value-at-risk",
    ]
    eq["exceedances"] = eq["exceedances"].astype(int)

    filt = agg.loc[["EWMA, filtered historical", "GARCH, filtered historical"]]
    others = agg["ratio"].drop(filt.index)
    _check(
        checks,
        "backtest",
        "filtered historical simulation is closest to the expected exceedance frequency",
        filt["ratio"].max() < others.min() and filt["ratio"].max() < 1.5,
        f"{filt['ratio'].iloc[0]:.2f} and {filt['ratio'].iloc[1]:.2f} times the expected number, others {others.min():.2f} to {others.max():.2f}",
        warn=fast,
    )
    _check(
        checks,
        "backtest",
        "the best method is within 20% of the expected frequency",
        filt["ratio"].min() < 1.2,
        f"{filt['ratio'].min():.2f} times the expected number: every method still under-forecasts on this sample",
        warn=True,
    )
    _check(
        checks,
        "backtest",
        "the normal method has too many exceedances",
        agg.loc["normal", "ratio"] > 1.4,
        f"{agg.loc['normal', 'ratio']:.2f} times the expected number",
    )
    _check(
        checks,
        "backtest",
        "filtering reduces rejections of the frequency test",
        filt["frequency rejected"].max()
        < min(agg.loc["normal", "frequency rejected"], agg.loc["historical simulation", "frequency rejected"]),
        f"{int(filt['frequency rejected'].max())} of {n_port} portfolios at most, against {int(agg.loc['normal', 'frequency rejected'])} (normal) and {int(agg.loc['historical simulation', 'frequency rejected'])} (historical)",
        warn=fast,
    )
    _check(
        checks,
        "backtest",
        "filtered expected shortfall is the best calibrated",
        filt["shortfall ratio"].sub(1).abs().max() <= agg["shortfall ratio"].drop(filt.index).sub(1).abs().min(),
        f"shortfall ratio {filt['shortfall ratio'].iloc[0]:.2f} and {filt['shortfall ratio'].iloc[1]:.2f}, others {agg['shortfall ratio'].drop(filt.index).min():.2f} to {agg['shortfall ratio'].drop(filt.index).max():.2f}",
        warn=True,
    )
    _check(
        checks,
        "backtest",
        "false-alarm rate of the tests is plausible for a good method",
        filt["frequency rejected"].max() <= max(3, 0.2 * n_port),
        f"{int(filt['frequency rejected'].max())} of {n_port} rejected at the 5% level",
        warn=True,
    )

    # 2. minimum-variance portfolios by covariance estimator
    mv_rows, cond = [], {}
    for n in sizes:
        r = engine.call("minvar", assets=n, window=cfg["minvar_window"], rebalance=cfg["rebalance"])
        cond[n] = {"sample covariance": r["condition_sample"], "shrunk covariance": r["condition_shrunk"]}
        for name, vol, gross, turn, intensity, failures in r["rows"]:
            mv_rows.append(
                {
                    "stocks": n,
                    "portfolio": LABELS[name],
                    "volatility": vol,
                    "gross exposure": gross,
                    "turnover": turn,
                    "intensity": intensity,
                    "failures": failures,
                }
            )
    mv = pd.DataFrame(mv_rows)
    vol = mv.pivot(index="stocks", columns="portfolio", values="volatility")[
        [LABELS[k] for k in ("equal_weight", "inverse_variance", "sample", "ledoit_wolf")]
    ]
    gross = mv.pivot(index="stocks", columns="portfolio", values="gross exposure")[
        [LABELS["sample"], LABELS["ledoit_wolf"]]
    ]
    turn = mv.pivot(index="stocks", columns="portfolio", values="turnover")[[LABELS["sample"], LABELS["ledoit_wolf"]]]
    s_col, l_col = LABELS["sample"], LABELS["ledoit_wolf"]
    gain = 1 - vol[l_col] / vol[s_col]
    _check(
        checks,
        "covariance",
        "shrinkage never increases realised volatility",
        bool((gain > -0.005).all()),
        "reduction " + ", ".join(f"{x:.1%}" for x in gain),
    )
    _check(
        checks,
        "covariance",
        "the gain grows as stocks approach the number of days",
        gain.iloc[-1] > gain.iloc[0] + 0.05,
        f"{gain.iloc[0]:.1%} with {sizes[0]} stocks, {gain.iloc[-1]:.1%} with {sizes[-1]}",
        warn=fast,
    )
    _check(
        checks,
        "covariance",
        "shrunk minimum variance beats the simple portfolios",
        bool((vol[l_col] < vol[LABELS["inverse_variance"]]).all()),
        f"largest universe: {vol[l_col].iloc[-1]:.1%} against {vol[LABELS['inverse_variance']].iloc[-1]:.1%} (inverse variance) and {vol[LABELS['equal_weight']].iloc[-1]:.1%} (equal weight)",
    )
    _check(
        checks,
        "covariance",
        "no matrix failed to factorise",
        int(mv["failures"].sum()) == 0,
        f"{int(mv['failures'].sum())} failures",
    )

    # 3. risk decomposition
    dec = engine.call("decompose", window=cfg["window"], top=8)
    top = pd.DataFrame(dec["rows"], columns=["stock", "share of portfolio risk", "own volatility"])
    _check(
        checks,
        "decomposition",
        "contributions add up to the portfolio volatility",
        abs(dec["sum_of_contributions"] / dec["volatility"] - 1) < 1e-10,
        f"relative gap {abs(dec['sum_of_contributions'] / dec['volatility'] - 1):.1e}",
    )

    plotting.overview(pd.read_csv(series), agg, vol, n_port, out / "figures" / "overview.png")
    series.unlink()

    val = pd.DataFrame(checks)
    counts = val["status"].value_counts().to_dict()
    L = [
        "# RiskCore report",
        "",
        f"Data: {int(desc['assets'])} stocks, {int(desc['days'])} daily returns. The equal-weight portfolio has an annualised volatility of {desc['volatility']:.1%}, "
        f"skewness {desc['skewness']:.2f}, excess kurtosis {desc['excess_kurtosis']:.1f}, worst day {desc['worst_day']:.1%}.",
        "",
        "## 1. Backtest of value-at-risk and expected shortfall",
        "",
        f"Every day each method sees the previous {cfg['window']} returns and forecasts the next day: value-at-risk at {cfg['var_level']:.0%}, expected shortfall at {cfg['es_level']:.1%}. "
        f"{days:,d} out-of-sample days, {n_port} portfolios (all stocks equally weighted, then {n_port - 1} random portfolios of {cfg['holdings']} stocks).",
        "",
        f"All portfolios together. Rejections are counts of portfolios, out of {n_port}, at the 5% level.",
        "",
        md_table(agg, 3),
        "",
        "Shortfall ratio: sum of loss / expected shortfall on exceedance days over the expected number of exceedances. Severity: mean of loss / expected shortfall on exceedance days. Both are 1 for a correct model.",
        "",
        "The equal-weight portfolio alone:",
        "",
        md_table(eq, 3),
        "",
        "## 2. Minimum-variance portfolios",
        "",
        f"Weights from the previous {cfg['minvar_window']} days, recomputed every {cfg['rebalance']} days, short positions allowed. Annualised out-of-sample volatility:",
        "",
        md_table(vol, 4),
        "",
        "Gross exposure (sum of absolute weights):",
        "",
        md_table(gross, 2),
        "",
        "Turnover per rebalance:",
        "",
        md_table(turn, 2),
        "",
        "Condition number of the first covariance matrix:",
        "",
        md_table(pd.DataFrame(cond).T.rename_axis("stocks"), 0),
        "",
        f"Mean shrinkage intensity: {mv.loc[mv['portfolio'] == l_col, 'intensity'].min():.3f} to {mv.loc[mv['portfolio'] == l_col, 'intensity'].max():.3f}.",
        "",
        "## 3. Risk decomposition of the equal-weight portfolio",
        "",
        f"Daily volatility {dec['volatility']:.4%} (shrunk covariance of the last {cfg['window']} days). Largest contributors, each holding 1/{int(desc['assets'])} of the capital:",
        "",
        md_table(top, 4, index=False),
        "",
        "## Validation",
        "",
        f"{counts.get('PASS', 0)} PASS, {counts.get('WARN', 0)} WARN, {counts.get('FAIL', 0)} FAIL. See `validation.md`.",
        "",
    ]
    (out / "report.md").write_text("\n".join(L))
    (out / "validation.md").write_text("# Validation checks\n\n" + md_table(val, index=False) + "\n")
    metrics = {
        "data": {k: v for k, v in desc.items() if k != "rows"},
        "backtest": agg.to_dict("index"),
        "equal_weight": eq.to_dict("index"),
        "minvar_volatility": {str(k): v for k, v in vol.to_dict("index").items()},
        "shrinkage_gain": {str(k): float(v) for k, v in gain.items()},
        "decomposition": {"volatility": dec["volatility"], "sum": dec["sum_of_contributions"]},
        "checks": checks,
    }
    (out / "metrics.json").write_text(json.dumps(metrics, indent=1, default=float))
    return metrics
