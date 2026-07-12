#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
VENV_DIR="$ROOT_DIR/.venv"
PYTHON_BIN="$VENV_DIR/bin/python"
REQ_PATH="$SCRIPT_DIR/requirements.txt"

cd "$ROOT_DIR"

if [[ ! -d "$VENV_DIR" ]]; then
  echo "Creating virtual environment at $VENV_DIR"
  python3 -m venv "$VENV_DIR"
fi

if [[ ! -x "$PYTHON_BIN" ]]; then
  echo "Virtual environment is missing its python executable: $PYTHON_BIN" >&2
  exit 1
fi

if [[ ! -f "$REQ_PATH" ]]; then
  echo "Generator requirements file not found: $REQ_PATH" >&2
  exit 1
fi

if ! "$PYTHON_BIN" -m pip install --disable-pip-version-check -r "$REQ_PATH"; then
  echo "Failed to install generator dependencies from $REQ_PATH" >&2
  exit 1
fi

GEN_ARGS=()

if [[ $# -gt 0 ]]; then
  GEN_ARGS+=("$@")
fi

has_arg() {
  local name="$1"
  shift || true

  local arg
  for arg in "$@"; do
    if [[ "$arg" == "$name" || "$arg" == "$name="* ]]; then
      return 0
    fi
  done

  return 1
}

if ! has_arg "--out" "${GEN_ARGS[@]+"${GEN_ARGS[@]}"}"; then
  GEN_ARGS+=(--out include/lunisolar_data.h)
fi

if ! has_arg "--test-vectors" "${GEN_ARGS[@]+"${GEN_ARGS[@]}"}" \
  && ! has_arg "--no-test-vectors" "${GEN_ARGS[@]+"${GEN_ARGS[@]}"}"; then
  GEN_ARGS+=(--test-vectors tests/lunisolar_vectors.inc)
fi

"$PYTHON_BIN" tools/gen_lunisolar_data.py "${GEN_ARGS[@]}"
