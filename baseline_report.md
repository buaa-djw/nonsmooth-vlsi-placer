# Stage 0 immutable baseline report

## Scope and provenance

- Baseline source commit: `0078a9e4266d23b77c08f7c05f1697b5b34cc532`.
- Baseline branch: `codex/paper-exact-reproduction` (created from the clean `work` branch).
- The initial worktree was clean (`git status --short --branch` reported only `## work`).
- No algorithm source was changed in this stage. This report is the only tracked baseline change.
- Compiler/configuration observed in the build: GNU C++ 13.3.0, C++17, CMake/Ninja, and zlib 1.3.
- Generated runs are retained locally under `output/baseline-stage0/`. This directory remains ignored and is not committed, in accordance with `AGENTS.md`.

## Build and test baseline

| Check | Result | Notes |
|---|---|---|
| Debug configure | PASS | `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` |
| Debug build | PASS with warnings | All 53 Ninja actions completed. Existing warnings include missing-field initializers, misleading indentation, and size-to-double conversions. |
| CTest | PASS | 16/16 tests passed in 0.25 s. |
| Release configure | PASS | `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release` |
| Release build | PASS | All 53 Ninja actions completed. |

## Baseline run commands

Tiny baseline:

```text
./build/nonsmooth_placer tests/data/tiny/tiny_basic/tiny_basic.aux \
  --seed 0 --iterations-per-stage 2 --penalty-stages 2 \
  --quadratic-iterations 2 --report-every 1 \
  --out output/baseline-stage0/tiny
```

Resource-bounded adaptec1 smoke baseline (one level, 16 x 16 bins, one quadratic and one optimizer iteration):

```text
./build/nonsmooth_placer testbench/ispd2005/adaptec1/adaptec1.aux \
  --expected-benchmark adaptec1 --seed 0 --max-levels 1 --bins 16 16 \
  --iterations-per-stage 1 --penalty-stages 1 --quadratic-iterations 1 \
  --report-every 1 --no-macro-shifting \
  --out output/baseline-stage0/adaptec1
```

The container has no `/usr/bin/time`; therefore peak resident memory is unavailable. Bash timing measured adaptec1 at 24.644 s wall, 22.062 s user, and 2.562 s system; the program-reported runtime was 24.2629 s.

## Metrics

These are **global-placement** metrics. They must not be compared with the paper's Table II final legalized/detailed-placement HPWL.

| Metric | tiny_basic | adaptec1 smoke |
|---|---:|---:|
| Cells / movable / fixed | 3 / 3 / 0 | 211447 / 210904 / 543 |
| Input nets / pins | 2 / 5 | 221142 / 944053 |
| Active level-0 nets / pins | 2 / 5 | 219794 / 942705 |
| Levels | 1 | 1 (forced smoke limit) |
| Iterations by level | L0: 2 | L0: 1 |
| Initial HPWL | 1.2649588807453496 | 1083429706.9169238 |
| Final global HPWL | 1.2649588807453496 | 1083429706.9169238 |
| Density penalty | 0.0073069534475406683 | 3511339631728.3125 |
| Paper OFR | 0.45579454177240702 | 0.12948416214601757 |
| Recorded objective (first row) | 2.0302992740225911 | 1206033742.9984334 |
| Runtime (program) | 0.0457695 s | 24.2629 s |
| Peak memory | unavailable | unavailable |
| Stop reason | `ofr_not_improved` | `safety_iteration_limit` |
| Output consistency | true | true |

Both output trees contain `final.pl`, `history.csv`, `summary.json`, `hierarchy.json`, `interlevel_hpwl.json`, stdout/stderr logs (in the adjacent baseline log directory for adaptec1), `run_info.json`, coarsest pre/post-quadratic PL files, and the level-0 final PL. With only one level, both `interlevel_hpwl.json` files are empty arrays. A scan of the generated JSON, CSV, PL, and logs found no NaN or Inf token.

## Artifact checksums

| Run | Artifact | SHA-256 |
|---|---|---|
| tiny | `final.pl` | `2dd95048f1294a859e7a0e2c57dce0e0ff61f2ef8e34fcc46e078b5241f5effd` |
| tiny | `history.csv` | `f542dd2c7a39a7da4e294a5a2ec21877e41ce0d58291485115db9a1b6133758c` |
| tiny | `summary.json` | `3d9e0ad9df78fdbd79546e68aef6abe70e3739f4848e8f9cf78cc1681f5d4e06` |
| tiny | `hierarchy.json` | `4b944d3f106435aff51bd8d639524e775ff60349b2ed36d3622093198e8d9102` |
| tiny | `interlevel_hpwl.json` | `37517e5f3dc66819f61f5a7bb8ace1921282415f10551d2defa5c3eb0985b570` |
| adaptec1 | `final.pl` | `e656f1fc641ed06a39bc18c0bd45df266fb766bca4770f1237948d03449c821d` |
| adaptec1 | `history.csv` | `49ad0258aadd896d2466d1952728923b51056285551173a9bfff11090cc66c8d` |
| adaptec1 | `summary.json` | `56caaa736f878e96521be65d7763872288f5af4e5cbcea5004e0765a9b900f90` |
| adaptec1 | `hierarchy.json` | `37cdbd4ab64e7df0eba7a78c45d3e18647c926a54b7e24231ed4e4fc0ccb31e0` |
| adaptec1 | `interlevel_hpwl.json` | `37517e5f3dc66819f61f5a7bb8ace1921282415f10551d2defa5c3eb0985b570` |

## Audit of the 18 reported issues

“Required” below means required for the stated paper-exact target, not fixed during Stage 0.

| # | Finding and evidence | Paper basis | Required / planned stage |
|---:|---|---|---|
| 1 | **Present.** Coarse-net construction uses a `set` keyed by coarse object ID and skips every later pin mapped to that object (`src/multilevel/Clusterer.cpp:49`). | Pin-preserving multilevel representation needed by Fig. 3 and HPWL Eq. (2). | Yes / Stage 6. |
| 2 | **Present.** After that deduplication, nets with fewer than two surviving coarse pins are omitted (`src/multilevel/Clusterer.cpp:49`), deleting cluster-internal nets. | Fig. 3 hierarchy; internal nets remain constant terms. | Yes / Stage 6. |
| 3 | **Present.** Interlevel checking pairs `c.nets[i]` with `f.nets[i]` (`src/objective/Wirelength.cpp:128-133`). | Eq. (2), checked net-by-net through Fig. 3. | Yes / Stage 6. |
| 4 | **Present.** `original_net_id` is copied while clustering (`src/multilevel/Clusterer.cpp:49`) but is not consulted by the index-based consistency loop (`src/objective/Wirelength.cpp:128-133`). | Eq. (2), Fig. 3. | Yes / Stage 6. |
| 5 | **Present.** Each merge recreates group mapping/connectivity and a fresh global priority queue, then erases a vector element (`src/multilevel/Clusterer.cpp:37-44`). | Modified best-choice flow, Fig. 2 and score Eq. (19). | Yes / Stage 6. |
| 6 | **Present.** Paper-L1 iterates every `i,j` pin pair (`src/objective/Wirelength.cpp:58-60`), which is O(p^2). | Eqs. (3), (6)-(8), with 2p-3 effective edges. | Yes / Stage 2. |
| 7 | **Present.** `sx` returns zero within `EPS`, including exact ties (`src/objective/Wirelength.cpp:7`), and is used directly for pair gradients (`src/objective/Wirelength.cpp:68-76`). | Tie subgradient Eq. (13). | Yes / Stage 2. |
| 8 | **Present.** Bin bounds subtract `EPS` at upper cell edges and overlap loops skip zero-overlap contacts (`src/objective/Density.cpp:19,22`), so boundary-contact stencil entries are discarded. | Exact overlap Eqs. (9)-(10), density subgradient. | Yes / Stage 3. |
| 9 | **Present.** The report-density constructor argument is explicitly ignored (`src/objective/Density.cpp:40`); OFR uses the penalty-density overflow (`src/objective/Density.cpp:29-38`). | Penalty Eq. (12), OFR Eq. (18). | Yes / Stage 3. |
| 10 | **Present.** Optimization constructs a grid from optional penalty/OFR densities (`app/main.cpp:69`), but final consistency creates a target-density-only grid (`app/main.cpp:89-92`). | Eqs. (12), (18); consistent evaluation state. | Yes / Stage 3. |
| 11 | **Present.** CLI parsing stores `s0`, `s-floor`, and delta values, but main passes none of them into `OptimizeConfig` (`app/main.cpp:70-79`); optimizer hard-codes the schedule and lambda multipliers (`src/optimizer/NonsmoothOptimizer.cpp:18,21`). | Fig. 1, Eq. (17), paper delta parameters. | Yes / Stage 5. |
| 12 | **Present.** Stage `best` is initialized from the current objective, then the first identical evaluation fails `improved` and increments stall (`src/optimizer/NonsmoothOptimizer.cpp:36,40-42`). Baseline history confirms first-row `no_improve_count=1`. | Fig. 1 inner stopping rule. | Yes / Stage 5. |
| 13 | **Present.** History records `alpha * direction_norm` before moving (`src/optimizer/NonsmoothOptimizer.cpp:47`); projection only increments a count and never records post-projection actual or projection displacement (`src/optimizer/NonsmoothOptimizer.cpp:50-51`). | Position update Eq. (17); auditability requirement. | Yes / Stage 5 (and projection semantics Stage 7). |
| 14 | **Present.** `res` is evaluated before Macro Shifting, then is stored without reevaluation (`app/main.cpp:81-85`). | Fig. 3 post-optimization flow and consistent metrics. | Yes / Stage 9. |
| 15 | **Present.** `WhitespaceAllocator.cpp` implements an allocator and CLI/config fields exist, but main neither includes nor calls it; the level flow only calls Macro Shifting (`app/main.cpp:81-86`). | Fig. 3 WSA position; NTUplace3 source/reference [12]. | Yes / Stage 10, after source audit. |
| 16 | **Present.** README says the C++ driver delegates full placement to Python (`README.md:9-12`), while `app/main.cpp:27-97` directly invokes native C++ modules. | Documentation/output provenance rather than a paper equation. | Yes / documentation stage (no algorithm change). |
| 17 | **Present as a reachable inconsistency.** Summary mixes live `exactHpwl(l)` with cached last optimizer density/OFR (`src/io/Reporter.cpp:10`); Macro Shifting can change the level afterward without refreshing cached metrics (`app/main.cpp:81-97`). The smoke configuration disabled shifting and passed consistency, but the default path can diverge or throw. | Eq. (2), Eq. (12), Eq. (18), same-state reporting. | Yes / Stage 9 and Stage 11. |
| 18 | **Present.** Concrete semantic differences include C++ using total fixed+movable overlap and no fixed-capacity subtraction (`src/objective/Density.cpp:28-40`), while Python constructs capacities by subtracting fixed area (`reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py:887-900`). C++ also deduplicates coarse pins as noted above. | Eqs. (9)-(12), (18), Figs. 2-3. The user-requested equations take priority where the reference currently conflicts. | Yes / addressed module-by-module, first Stage 1 then Stages 2-10. |

## Invariants and limitations

- Debug/Release builds and all current tests pass.
- The two captured runs completed without NaN/Inf and reported round-trip consistency `true`.
- The adaptec1 smoke retained all 543 fixed objects; the output consistency maximum coordinate difference was approximately `9.09e-13` across reloaded coordinates.
- The active adaptec1 level has 1,348 fewer nets and incidences than the input database (221142/944053 versus 219794/942705). This is existing level-0 filtering behavior and must be audited before claiming “no lost nets/pins.”
- No coarse/fine continuity claim can be made from these resource-bounded baselines because each deliberately contains one level and therefore has no interlevel pair.
- Determinism was not established by a second full rerun; the seed and all baseline parameters are recorded for later byte/numeric replay.
- Peak memory could not be captured because GNU `time` is absent from the environment.
- Legalization, detailed placement, look-ahead legalization, and paper-sourced WSA are explicitly unreproduced. These are global-placement results only.

## Stage 0 gate

**PASS** for establishing and documenting the current baseline. No permission is implied to start Stage 1 until the user confirms.
