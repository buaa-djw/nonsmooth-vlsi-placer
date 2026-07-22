#!/usr/bin/env python3
import json, pathlib, sys
root=pathlib.Path(__file__).resolve().parents[1]
py=root/'output/python/adaptec1_smoke/summary.json'; cp=root/'output/cpp/adaptec1_smoke/summary.json'
if not py.exists() or not cp.exists():
    print('warning: expected adaptec1 outputs not found; run scripts first')
    sys.exit(0)
a=json.loads(py.read_text()); b=json.loads(cp.read_text())
assert len(a.get('levels',[]))==len(b.get('levels',[]))
print('basic schema comparison passed')
