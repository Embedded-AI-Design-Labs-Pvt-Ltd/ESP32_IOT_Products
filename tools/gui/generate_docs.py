#!/usr/bin/env python3
"""Copy GUI HTML into docs/html for Jenkins artifact publishing."""
from pathlib import Path
import shutil

root = Path(__file__).resolve().parents[2]
src = root / "tools" / "gui" / "static"
dst = root / "docs" / "html"
dst.mkdir(parents=True, exist_ok=True)
for p in src.iterdir():
    if p.is_file():
        shutil.copy2(p, dst / p.name)
print("docs/html synchronized from tools/gui/static")
