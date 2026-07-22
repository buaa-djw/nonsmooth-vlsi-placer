#!/usr/bin/env bash
set -euo pipefail
./build/nonsmooth_placer testbench/ispd2005/adaptec1/adaptec1.aux --target-density 1.0 --current 150 --cluster-degree-cap 256 --penalty-stages 1 --iterations-per-stage 5 --nmax 1000000 --report-every 1 --out output/cpp/adaptec1_smoke
