import pandas as pd

from riskcore.report import md_table


def test_markdown_table():
    text = md_table(pd.DataFrame({"a": [1, 2], "b": [0.5, 1e-9]}, index=["x", "y"]), 3)
    assert text.splitlines()[1].count("---") == 3 and "1.00e-09" in text and "| x | 1 | 0.500 |" in text
