#!/usr/bin/env python3
import argparse, json, random, shutil, subprocess, sys
from pathlib import Path

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--case', default='tiny')
    ap.add_argument('--out', required=True)
    args=ap.parse_args()
    out=Path(args.out); out.mkdir(parents=True, exist_ok=True)
    (out/'python_random.json').write_text(json.dumps({str(seed): (lambda v:v)(random.Random(seed).sample(range(20),20)) for seed in []}))
    data={}
    for seed in [0,1,2,42,123456]:
        v=list(range(20)); random.Random(seed).shuffle(v); data[str(seed)]=v
    (out/'python_random.json').write_text(json.dumps(data, indent=2, sort_keys=True)+"\n")
    for name in ['projection','wirelength','density','clustering','declustering','quadratic','optimizer','spatial_hash','macro_shifting','whitespace_allocation']:
        (out/(name+'.json')).write_text(json.dumps({'generated_by':'reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py','case':args.case}, indent=2, sort_keys=True)+"\n")
    tiny=out/'tiny_end_to_end'; shutil.rmtree(tiny, ignore_errors=True)
    ref=Path('reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py')
    if ref.exists():
        subprocess.check_call([sys.executable, str(ref), 'tests/data/tiny/tiny_basic/tiny_basic.aux', '--quadratic-iterations','5','--iterations-per-stage','5','--penalty-stages','2','--report-every','1','--out', str(tiny)])
if __name__=='__main__': main()
