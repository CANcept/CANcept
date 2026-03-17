#!/usr/bin/env python3
"""Add Apache 2.0 license header to all C++ source files in src/ and tests/."""

from pathlib import Path

HEADER = """\
/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"""

ROOT = Path(__file__).parent.parent

def process(path: Path) -> bool:
    content = path.read_text(encoding="utf-8")
    if content.startswith("/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier"):
        return False
    path.write_text(HEADER + "\n" + content, encoding="utf-8")
    return True

added = skipped = 0
for directory in ("src", "tests"):
    for ext in ("*.cpp", "*.hpp"):
        for file in (ROOT / directory).rglob(ext):
            if process(file):
                print(f"  + {file.relative_to(ROOT)}")
                added += 1
            else:
                print(f"  - {file.relative_to(ROOT)}")
                skipped += 1

print(f"\nDone: {added} headers added, {skipped} files already had one.")
