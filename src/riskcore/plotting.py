"""Static figures for the report (matplotlib, PNG for the README)."""

from __future__ import annotations

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

# Validated categorical order (fixed, never cycled) + recessive ink.
SERIES = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4", "#008300", "#6250d6", "#e34948"]
SURFACE, INK, INK2, GRID = "#fcfcfb", "#0b0b0b", "#52514e", "#e4e3df"
NEUTRAL = "#8a8985"
CRITICAL = "#c4271a"

plt.rcParams.update(
    {
        "figure.facecolor": SURFACE,
        "axes.facecolor": SURFACE,
        "savefig.facecolor": SURFACE,
        "axes.edgecolor": GRID,
        "axes.labelcolor": INK2,
        "xtick.color": INK2,
        "ytick.color": INK2,
        "text.color": INK,
        "axes.grid": True,
        "grid.color": GRID,
        "grid.linewidth": 0.6,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "font.size": 9,
        "axes.titlesize": 10,
        "axes.titleweight": "bold",
        "legend.frameon": False,
        "lines.linewidth": 1.6,
        "figure.dpi": 150,
        "axes.axisbelow": True,
    }
)


def _save(fig, path):
    fig.tight_layout()
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)


def overview(series, agg, vol, n_port, path):
    fig, ax = plt.subplots(2, 2, figsize=(11, 7.6))
    a = ax[0, 0]
    d = series["day"]
    a.plot(d, 100 * series["loss"], color=NEUTRAL, lw=0.5, label="daily loss")
    for i, (col, label) in enumerate(
        [("historical", "historical simulation"), ("ewma_filtered", "EWMA, filtered historical")]
    ):
        a.plot(d, 100 * series[col], color=SERIES[i], lw=1.3, label=label)
        hit = series["loss"] > series[col]
        a.plot(d[hit], 100 * series.loc[hit, "loss"], "o" if i == 0 else "x", color=SERIES[i], ms=4 + i)
    a.set(title="Equal-weight portfolio: 99% value-at-risk and exceedances", xlabel="out-of-sample day", ylabel="%")
    a.legend(fontsize=7)

    y = np.arange(len(agg))[::-1]
    a = ax[0, 1]
    a.barh(y, agg["ratio"], height=0.6, color=SERIES[0])
    a.axvline(1.0, color=INK2, lw=1, ls="--")
    for yi, (r, k) in zip(y, zip(agg["ratio"], agg["frequency rejected"], strict=True), strict=True):
        a.text(r + 0.02, yi, f"{r:.2f}  ({int(k)}/{n_port} rejected)", va="center", fontsize=7, color=INK2)
    a.set_yticks(y, agg.index)
    a.set_xlim(0, agg["ratio"].max() * 1.45)
    a.set(title="Exceedances over expected, all portfolios", xlabel="ratio (1 is correct)")
    a.grid(axis="y", visible=False)

    a = ax[1, 0]
    a.barh(y, agg["shortfall ratio"], height=0.6, color=SERIES[1])
    a.axvline(1.0, color=INK2, lw=1, ls="--")
    for yi, r in zip(y, agg["shortfall ratio"], strict=True):
        a.text(r + 0.02, yi, f"{r:.2f}", va="center", fontsize=7, color=INK2)
    a.set_yticks(y, agg.index)
    a.set_xlim(0, agg["shortfall ratio"].max() * 1.2)
    a.set(title="Expected shortfall: realised over forecast", xlabel="shortfall ratio (1 is correct)")
    a.grid(axis="y", visible=False)

    a = ax[1, 1]
    for i, col in enumerate(vol.columns):
        a.plot(vol.index, 100 * vol[col], "o-", color=SERIES[i], ms=4, label=col)
    a.set(
        title="Out-of-sample volatility against universe size",
        xlabel="number of stocks (250-day window)",
        ylabel="annualised volatility, %",
    )
    a.legend(fontsize=7)
    _save(fig, path)
