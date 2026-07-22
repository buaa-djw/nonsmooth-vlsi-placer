# nonsmooth-vlsi-placer

This repository keeps the original single-file Python nonsmooth multilevel VLSI global placer as the algorithmic reference and adds a C++17 project layout for migration and regression.

## Python reference

The reference implementation is `reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py`. It remains the source of truth for Bookshelf parsing, multilevel clustering/declustering, exact HPWL, density penalty, quadratic initialization, nonsmooth optimization, Macro Shifting, and output schema.

## C++ status

The C++ executable (`build/nonsmooth_placer`) provides the modular project scaffolding, C++ Bookshelf database/parser, level-0 representation, HPWL helper, writers, and tests. For end-to-end numerical equivalence during this migration slice, the C++ driver validates the input through the C++ parser and delegates the full placement run to the Python reference with the same command-line arguments.

## Dependencies

Ubuntu 22.04, GCC 11+, CMake 3.16+, optional Ninja, Python 3, and zlib. No network-downloaded dependencies are used.

## Build and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/nonsmooth_placer testbench/ispd2005/adaptec1/adaptec1.aux --target-density 1.0 --current 150 --cluster-degree-cap 256 --penalty-stages 1 --iterations-per-stage 5 --nmax 1000000 --report-every 1 --out output/cpp/adaptec1_smoke
```

Outputs match the Python file names: `hierarchy.json`, `coarsest_before_quadratic.pl`, `coarsest_after_quadratic.pl`, `history.csv`, `interlevel_hpwl.json`, `summary.json`, `run_info.json`, `level_<index>_final.pl`, and `final.pl`.

## Global placement vs legalization

The placer emits a global placement. It optimizes continuous locations and does not guarantee row/site legalization or overlap-free standard-cell placement; use an external legalizer afterward.

## Regression

```bash
./scripts/run_python_adaptec1.sh
./scripts/run_cpp_adaptec1.sh
python3 ./scripts/compare_python_cpp.py
```

If ISPD2005 data is absent, use the tiny fixtures under `tests/data/tiny` and CTest.
