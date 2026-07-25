#!/usr/bin/env bash
set -euo pipefail
ITERATIONS_PER_STAGE=${ITERATIONS_PER_STAGE:-2000}
PENALTY_STAGES=${PENALTY_STAGES:-20}
NMAX=${NMAX:-0}
QUADRATIC_ITERATIONS=${QUADRATIC_ITERATIONS:-200}
REPORT_EVERY=${REPORT_EVERY:-100}
TARGET_DENSITY=${TARGET_DENSITY:-1.0}
CURRENT=${CURRENT:-150}
RANDOM_SEED=${RANDOM_SEED:-1}
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUT="output/cpp/adaptec1_${TIMESTAMP}"
./build/nonsmooth_placer testbench/ispd2005/adaptec1/adaptec1.aux \
  --expected-benchmark adaptec1 --target-density "${TARGET_DENSITY}" --current "${CURRENT}" \
  --penalty-stages "${PENALTY_STAGES}" --iterations-per-stage "${ITERATIONS_PER_STAGE}" \
  --quadratic-iterations "${QUADRATIC_ITERATIONS}" --nmax "${NMAX}" \
  --report-every "${REPORT_EVERY}" --seed "${RANDOM_SEED}" --out "${OUT}"
echo "Result directory: ${OUT}"
echo "History: ${OUT}/history.csv"
echo "Summary: ${OUT}/summary.json"
echo "Placement: ${OUT}/final.pl"
