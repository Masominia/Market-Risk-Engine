import pytest

from riskcore import engine
from riskcore.pipeline import run


@pytest.mark.skipif(not engine.available(), reason="C++ engine not built")
def test_fast_pipeline(tmp_path):
    m = run(out=tmp_path, fast=True)
    assert not [c for c in m["checks"] if c["status"] == "FAIL"]
    assert (tmp_path / "report.md").exists() and (tmp_path / "figures" / "overview.png").exists()
    assert not (tmp_path / "var_series.csv").exists()
