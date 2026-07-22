#!/usr/bin/env bash
set -euo pipefail
ITERATIONS_PER_STAGE=${ITERATIONS_PER_STAGE:-5}
QUADRATIC_ITERATIONS=${QUADRATIC_ITERATIONS:-200}
REPORT_EVERY=${REPORT_EVERY:-1}
OUTPUT_ROOT=${OUTPUT_ROOT:-output/cpp}
./build/nonsmooth_placer testbench/ispd2005/adaptec1/adaptec1.aux --target-density 1.0 --current 150 --cluster-degree-cap 256 --penalty-stages 1 --iterations-per-stage "$ITERATIONS_PER_STAGE" --quadratic-iterations "$QUADRATIC_ITERATIONS" --nmax 1000000 --report-every "$REPORT_EVERY" --out "$OUTPUT_ROOT/adaptec1_smoke"
