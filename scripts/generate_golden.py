#!/usr/bin/env python3
import json, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'tests/golden/tiny_basic_python'
subprocess.check_call([sys.executable,str(root/'reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py'),str(root/'tests/data/tiny/tiny_basic/tiny_basic.aux'),'--penalty-stages','1','--iterations-per-stage','2','--out',str(out)])
print(out)
