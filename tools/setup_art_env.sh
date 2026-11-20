#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
venv_path="$repo_root/.venv-art"

python3 -m venv "$venv_path"
"$venv_path/bin/python" -m pip install --upgrade pip
"$venv_path/bin/python" -m pip install -r "$repo_root/tools/requirements-art.txt"

echo "Art environment ready. Use: $venv_path/bin/python tools/art_pipeline.py --help"
