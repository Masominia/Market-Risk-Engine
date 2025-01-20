import numpy as np
import pandas as pd
import pytest

from riskcore import engine

pytestmark = pytest.mark.skipif(not engine.available(), reason="C++ engine not built")


def _panel(tmp_path, days=700, assets=6, seed=0):
    rng = np.random.default_rng(seed)
    vol = np.where(np.arange(assets) < assets // 2, 0.005, 0.02)
    prices = 100 * np.exp(np.cumsum(vol * rng.standard_normal((days, assets)), axis=0))
    path = tmp_path / "prices.csv"
    pd.DataFrame(prices, columns=[f"S{i}" for i in range(assets)]).rename_axis("day").to_csv(path)
    return path, prices


def test_describe_matches_numpy(tmp_path):
    path, prices = _panel(tmp_path)
    r = (prices[1:] / prices[:-1] - 1).mean(axis=1)
    d = engine.call("describe", path)
    assert d["days"] == len(r) and d["assets"] == 6
    assert abs(d["volatility"] - r.std(ddof=1) * np.sqrt(252)) < 1e-9
    assert abs(d["worst_day"] - r.min()) < 1e-9


def test_backtest_on_normal_returns(tmp_path):
    path, _ = _panel(tmp_path, days=1500)
    out = tmp_path / "series.csv"
    b = engine.call("backtest", path, portfolios=2, holdings=3, out=out)
    assert b["out_of_sample_days"] == 1249 and len(b["rows"]) == 14
    for _, method, exceed, *_rest in b["rows"]:
        assert 3 <= exceed <= 30, method  # 12.5 expected
    s = pd.read_csv(out)
    assert len(s) == 1249 and list(s.columns[:3]) == ["day", "loss", "historical"]
    assert int((s["loss"] > s["historical"]).sum()) == b["rows"][0][2]


def test_minimum_variance_prefers_quiet_assets(tmp_path):
    path, _ = _panel(tmp_path)
    rows = {r[0]: r for r in engine.call("minvar", path)["rows"]}
    assert rows["sample"][1] < 0.6 * rows["equal_weight"][1]
    assert rows["ledoit_wolf"][1] < 0.6 * rows["equal_weight"][1] and rows["ledoit_wolf"][5] == 0


def test_real_panel_is_readable():
    d = engine.call("describe")
    assert d["assets"] == 300 and d["days"] == 1361
    dec = engine.call("decompose", top=3)
    assert abs(dec["sum_of_contributions"] / dec["volatility"] - 1) < 1e-10 and len(dec["rows"]) == 3


def test_bad_input_is_reported(tmp_path):
    bad = tmp_path / "bad.csv"
    bad.write_text("day,A,B\n0,1\n")
    with pytest.raises(Exception):  # noqa: B017
        engine.call("describe", bad)
