#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")" && pwd)"; cd "$root"
aux=testbench/ispd2005/adaptec2/adaptec2.aux
[[ -f "$aux" ]] || { echo "missing $aux" >&2; exit 2; }
stamp="$(date -u +%Y%m%dT%H%M%SZ)"; out="${OUTPUT_ROOT:-output/paper}/adaptec2_$stamp"
exec ./build/nonsmooth_placer "$aux" --expected-benchmark adaptec2 --wirelength-mode paper_b2b --current 150 --s0 0.2 --s-floor 0.06 --delta1 1.6 --delta2 1.9 --delta3 2.2 --penalty-stages 20 --iterations-per-stage 10000 --nmax paper --seed "${SEED:-1}" --out "$out"
