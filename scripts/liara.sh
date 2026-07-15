#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if ! command -v python3 >/dev/null; then
    echo "ERROR: python3 is required to run Liara orchestrator." >&2
    exit 1
fi

python3 "${SCRIPT_DIR}/liara.py" "$@"