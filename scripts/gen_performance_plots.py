#!/usr/bin/env python3
"""
Generate an HTML page with performance box plots from Google Benchmark JSON output.

Usage:
    python3 gen_performance_plots.py [benchmark.json] [output.html]

Defaults:
    benchmark.json -> build/benchmark_results.json
    output.html    -> doc/test/performance_plots.html

Requirements:
    pip install plotly
"""

import json
import os
import shutil
import sys
from collections import defaultdict
from pathlib import Path

import plotly
import plotly.graph_objects as go
from plotly.subplots import make_subplots


def _copy_plotly_min(output_dir: Path) -> None:
    """Copy plotly.min.js from the installed package next to the output file."""
    src = Path(plotly.__file__).parent / "package_data" / "plotly.min.js"
    if src.exists():
        shutil.copy(src, output_dir / "plotly.min.js")


PLOTS_PER_ROW = 2
ACCENT_COLOR   = "#bd93f9"
FILL_COLOR     = "rgba(189,147,249,0.2)"


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def parse_benchmark(filepath: str) -> tuple[dict[str, list[float]], str]:
    """Return ({display_name: [real_time, ...]}, time_unit) from a benchmark JSON."""
    with open(filepath, encoding="utf-8") as fh:
        data = json.load(fh)

    groups: dict[str, list[float]] = defaultdict(list)
    time_unit = "ns"

    for entry in data.get("benchmarks", []):
        if entry.get("run_type") != "iteration":
            continue
        name = entry.get("run_name", entry.get("name", "unknown"))
        name = name[3:] if name.startswith("BM_") else name
        groups[name].append(entry.get("real_time", entry.get("cpu_time", 0.0)))
        time_unit = entry.get("time_unit", time_unit)

    return dict(groups), time_unit


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

def generate_html(json_path: str, output_path: str) -> None:
    groups, time_unit = parse_benchmark(json_path)
    if not groups:
        print("No benchmark iteration records found in the JSON.")
        sys.exit(1)

    names = list(groups.keys())
    n     = len(names)
    cols  = min(PLOTS_PER_ROW, n)
    rows  = (n + cols - 1) // cols

    fig = make_subplots(
        rows=rows,
        cols=cols,
        subplot_titles=names,
        horizontal_spacing=0.08,
        vertical_spacing=0.06,
    )

    for i, name in enumerate(names):
        r = i // cols + 1
        c = i % cols + 1
        fig.add_trace(
            go.Box(
                y=groups[name],
                name=name,
                boxpoints="all",
                jitter=0.4,
                pointpos=0,
                marker=dict(size=3, color=ACCENT_COLOR),
                line=dict(color=ACCENT_COLOR),
                fillcolor=FILL_COLOR,
                showlegend=False,
            ),
            row=r,
            col=c,
        )
        fig.update_yaxes(title_text=time_unit, row=r, col=c)

    fig.update_layout(
        height=320 * rows,
        paper_bgcolor="rgba(0,0,0,0)",
        plot_bgcolor="rgba(0,0,0,0)",
        modebar=dict(bgcolor="rgba(0,0,0,0)"),
        margin=dict(t=60, l=10, r=10, b=10),
    )

    _copy_plotly_min(Path(output_path).parent)
    fig.write_html(output_path, include_plotlyjs="directory", full_html=False)
    print(f"Performance plots written → {output_path}  ({n} benchmarks, {time_unit})")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    json_file   = sys.argv[1] if len(sys.argv) > 1 else "build/benchmark_results.json"
    output_file = sys.argv[2] if len(sys.argv) > 2 else "doc/test/performance_plots.html"

    if not os.path.isfile(json_file):
        print(f"Error: benchmark JSON not found: {json_file}")
        print("Run PerformanceTests with --benchmark_out first.")
        sys.exit(1)

    Path(output_file).parent.mkdir(parents=True, exist_ok=True)
    generate_html(json_file, output_file)
