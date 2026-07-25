# Agent instructions

The Python file under `reference/python/` is the sole algorithmic source of truth. Do not replace exact HPWL, density, clustering, declustering, quadratic initialization, nonsmooth optimization, macro shifting, or WSA compatibility behavior with different algorithms.

Use deterministic containers/traversal; never depend on unordered iteration for numerical behavior. Preserve `EPS = 1.0e-12`, use `double`, and do not enable `-ffast-math`/`-Ofast`.

Build/test:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Directory responsibilities: `include/placer` public C++ API, `src` implementation, `app` executable, `reference/python` immutable reference, `tests/data/tiny` tiny Bookshelf fixtures, `scripts` regression helpers, `docs` mapping/observations.

Numerical targets: topology exact; region <=1e-12; initial HPWL <=1e-12 relative; density gradient <=1e-10 absolute; child offsets <=1e-12; interlevel HPWL relative delta <=1e-8; final HPWL <=1e-6 relative; final OFR <=1e-6; `.pl` coordinates six decimals or <=1e-6.

Do not modify ISPD benchmark contents under `testbench/ispd2005`. Do not commit generated `build/`, `output/`, logs, temporary files, or `__pycache__/`. Output schemas and file names must remain Python-compatible.
