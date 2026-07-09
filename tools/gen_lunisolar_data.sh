#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_DIR="$ROOT_DIR/.venv"
PYTHON_BIN="$VENV_DIR/bin/python"

cd "$ROOT_DIR"

if [[ ! -d "$VENV_DIR" ]]; then
  echo "Creating virtual environment at $VENV_DIR"
  python3 -m venv "$VENV_DIR"
fi

if [[ ! -x "$PYTHON_BIN" ]]; then
  echo "Virtual environment is missing its python executable: $PYTHON_BIN" >&2
  exit 1
fi

"$PYTHON_BIN" -m pip install --upgrade pip

if ! "$PYTHON_BIN" -c "import sxtwl" >/dev/null 2>&1; then
  echo "Installing sxtwl into $VENV_DIR"
  "$PYTHON_BIN" -m pip install sxtwl
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
