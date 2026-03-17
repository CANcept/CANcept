#!/usr/bin/env python3
"""
Generate a standalone HTML Treemap from an LCOV coverage report.

Usage:
    python3 coverage_treemap.py [coverage.info] [output.html]

Defaults:
    coverage.info  -> build/coverage.info
    output.html    -> coverage_treemap.html

Requirements:
    pip install plotly
"""

import os
import shutil
import sys
from pathlib import Path

import plotly
import plotly.graph_objects as go


def _copy_plotly_min(output_dir: Path) -> None:
    """Copy plotly.min.js from the installed package next to the output file."""
    src = Path(plotly.__file__).parent / "package_data" / "plotly.min.js"
    if src.exists():
        shutil.copy(src, output_dir / "plotly.min.js")


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def parse_lcov(filepath: str) -> dict[str, tuple[int, int]]:
    """Return {file_path: (lines_found, lines_hit)} for every record in the LCOV file."""
    files: dict[str, tuple[int, int]] = {}
    current_file: str | None = None
    lf = lh = 0

    with open(filepath, encoding="utf-8") as fh:
        for raw in fh:
            line = raw.strip()
            if line.startswith("SF:"):
                current_file = line[3:]
                lf = lh = 0
            elif line.startswith("LF:"):
                lf = int(line[3:])
            elif line.startswith("LH:"):
                lh = int(line[3:])
            elif line == "end_of_record":
                if current_file and lf > 0:
                    files[current_file] = (lf, lh)
                current_file = None

    return files


# ---------------------------------------------------------------------------
# Tree building
# ---------------------------------------------------------------------------

def _strip_prefix(files: dict[str, tuple[int, int]]) -> dict[str, tuple[int, int]]:
    """Shorten absolute paths to start from the 'src/' directory."""
    result: dict[str, tuple[int, int]] = {}
    for path, value in files.items():
        # Keep everything from 'src/' onwards
        idx = path.find("/src/")
        short = path[idx + 1:] if idx >= 0 else path
        result[short] = value
    return result


def build_tree(
    files: dict[str, tuple[int, int]],
) -> tuple[list, list, list, list, list, list]:
    """
    Build parallel lists for go.Treemap from the file coverage dict.

    Returns: ids, labels, parents, values, coverages, hover_texts
    """
    # Accumulate directories by summing all files beneath them
    dir_totals: dict[str, list[int]] = {}  # path -> [lf_sum, lh_sum]

    for filepath, (lf, lh) in files.items():
        parts = Path(filepath).parts
        for depth in range(1, len(parts)):
            dir_path = str(Path(*parts[:depth]))
            if dir_path not in dir_totals:
                dir_totals[dir_path] = [0, 0]
            dir_totals[dir_path][0] += lf
            dir_totals[dir_path][1] += lh

    # ---- root node ----
    total_lf = sum(v[0] for v in files.values())
    total_lh = sum(v[1] for v in files.values())
    root_cov = (total_lh / total_lf * 100) if total_lf > 0 else 0.0

    ids      = ["__root__"]
    labels   = [f"Total  {root_cov:.1f} %"]
    parents  = [""]
    values   = [total_lf]
    covs     = [root_cov]
    hovers   = [f"{total_lh} / {total_lf} lines  ({root_cov:.1f} %)"]

    def _parent_id(path_str: str) -> str:
        parts = Path(path_str).parts
        return str(Path(*parts[:-1])) if len(parts) > 1 else "__root__"

    # ---- directory nodes ----
    for dir_path, (dlf, dlh) in sorted(dir_totals.items()):
        cov = (dlh / dlf * 100) if dlf > 0 else 0.0
        ids.append(dir_path)
        labels.append(Path(dir_path).name)
        parents.append(_parent_id(dir_path))
        values.append(dlf)
        covs.append(cov)
        hovers.append(f"{dlh} / {dlf} lines  ({cov:.1f} %)")

    # ---- file nodes ----
    for filepath, (lf, lh) in sorted(files.items()):
        cov = (lh / lf * 100) if lf > 0 else 0.0
        ids.append(filepath)
        labels.append(Path(filepath).name)
        parents.append(_parent_id(filepath))
        values.append(lf)
        covs.append(cov)
        hovers.append(f"{lh} / {lf} lines  ({cov:.1f} %)")

    return ids, labels, parents, values, covs, hovers


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

COLORSCALE = [
    [0.00, "#b71c1c"],   # 0 %   deep red
    [0.50, "#e65100"],   # 50 %  orange
    [0.75, "#f9a825"],   # 75 %  amber
    [0.90, "#2e7d32"],   # 90 %  green
    [1.00, "#1b5e20"],   # 100 % dark green
]


def generate_html(lcov_path: str, output_path: str) -> None:
    files = parse_lcov(lcov_path)
    if not files:
        print("No coverage records found — is the LCOV file populated?")
        sys.exit(1)

    files = _strip_prefix(files)
    ids, labels, parents, values, covs, hovers = build_tree(files)

    fig = go.Figure(
        go.Treemap(
            ids=ids,
            labels=labels,
            parents=parents,
            values=values,
            branchvalues="total",
            customdata=hovers,
            hovertemplate="<b>%{label}</b><br>%{customdata}<extra></extra>",
            texttemplate="<b>%{label}</b><br>%{customdata}",
            textfont=dict(size=12),
            marker=dict(
                colors=covs,
                colorscale=COLORSCALE,
                cmin=0,
                cmax=100,
                showscale=True,
                colorbar=dict(
                    title=dict(text="Coverage %", side="right"),
                    ticksuffix="%",
                    thickness=16,
                ),
            ),
        )
    )

    total_lf = sum(v[0] for v in files.values())
    total_lh = sum(v[1] for v in files.values())
    overall  = (total_lh / total_lf * 100) if total_lf > 0 else 0.0

    fig.update_layout(
        title=dict(
            text=f"Total Coverage: {overall:.1f} %  ({total_lh}/{total_lf} lines)",
            font=dict(size=16),
        ),
        margin=dict(t=60, l=10, r=10, b=10),
        paper_bgcolor="rgba(0,0,0,0)",
        plot_bgcolor="rgba(0,0,0,0)",
        modebar=dict(bgcolor="rgba(0,0,0,0)"),
    )

    _copy_plotly_min(Path(output_path).parent)
    fig.write_html(output_path, include_plotlyjs="directory", full_html=False)
    print(f"Treemap written → {output_path}  ({len(files)} files, {overall:.1f} % overall)")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    lcov_file   = sys.argv[1] if len(sys.argv) > 1 else "../build/coverage.info"
    output_file = sys.argv[2] if len(sys.argv) > 2 else "../doc/test/coverage_treemap.html"

    if not os.path.isfile(lcov_file):
        print(f"Error: LCOV file not found: {lcov_file}")
        print("Run './start.sh -cov' first to generate coverage data.")
        sys.exit(1)

    Path(output_file).parent.mkdir(parents=True, exist_ok=True)
    generate_html(lcov_file, output_file)
