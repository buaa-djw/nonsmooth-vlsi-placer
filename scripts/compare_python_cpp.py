#!/usr/bin/env python3
import argparse, csv, json, math, sys
from pathlib import Path

def load_json(p):
    if not p.exists():
        print(f"missing file: {p}"); sys.exit(1)
    return json.loads(p.read_text())
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--python-out', default='output/python/adaptec1_smoke')
    ap.add_argument('--cpp-out', default='output/cpp/adaptec1_smoke')
    ap.add_argument('--rel', type=float, default=1e-8)
    args=ap.parse_args(); py=Path(args.python_out); cp=Path(args.cpp_out)
    files=['hierarchy.json','interlevel_hpwl.json','summary.json','history.csv','final.pl']
    for d in [py,cp]:
        for f in files:
            if not (d/f).exists(): print(f"missing file: {d/f}"); return 1
    bad=0
    print(f"{'metric':30s} {'python':>16s} {'cpp':>16s} {'abs_error':>16s} {'rel_error':>16s} status")
    def cmp(name,a,b,tol=args.rel):
        nonlocal bad
        ae=abs(float(a)-float(b)); re=ae/max(1e-12,abs(float(a))); ok=re<=tol or ae<=tol
        print(f"{name:30s} {a:16.8g} {b:16.8g} {ae:16.8g} {re:16.8g} {'PASS' if ok else 'FAIL'}")
        bad += 0 if ok else 1
    hjp,hjc=load_json(py/'hierarchy.json'),load_json(cp/'hierarchy.json')
    if len(hjp.get('levels',[]))!=len(hjc.get('levels',[])):
        print('hierarchy.level_count mismatch'); bad+=1
    for i,(a,b) in enumerate(zip(hjp.get('levels',[]),hjc.get('levels',[]))):
        for k in ['index','objects','movable','macros','nets']:
            if a.get(k)!=b.get(k): print(f'L{i}.{k} {a.get(k)} != {b.get(k)}'); bad+=1
    sp,sc=load_json(py/'summary.json'),load_json(cp/'summary.json')
    for k in ['final_hpwl']:
        if k in sp and k in sc: cmp(k,sp[k],sc[k],1e-6)
    return 1 if bad else 0
if __name__=='__main__': sys.exit(main())
