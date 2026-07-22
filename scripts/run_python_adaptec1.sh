#!/usr/bin/env bash
set -euo pipefail
python3 reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py testbench/ispd2005/adaptec1/adaptec1.aux --target-density 1.0 --current 150 --cluster-degree-cap 256 --penalty-stages 1 --iterations-per-stage 5 --nmax 1000000 --report-every 1 --out output/python/adaptec1_smoke
