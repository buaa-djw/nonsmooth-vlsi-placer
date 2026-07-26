# Stage 2 wirelength report

## Provenance and scope

- Algorithm baseline: `0078a9e4266d23b77c08f7c05f1697b5b34cc532`.
- Stage 0: `835ce1089175f94e93f910961f725c5e100a00ae`.
- Stage 1 and Stage 2 parent: `8753b66f90e539143eff0248dff65653c8a0aa29`.
- Branch: `codex/paper-exact-reproduction`.
- Changed only Wirelength API/implementation, wirelength tests, the minimal optimizer evaluator/seed wiring, and this report. Density, Clusterer, Level, database, I/O, postprocess, application, Python reference, and benchmarks are unchanged.
- `NonsmoothOptimizer.hpp` only adds `wire_seed`; `NonsmoothOptimizer.cpp` creates one level-local `WirelengthEvaluator` and replaces stateless wire-gradient calls with it. PR beta, direction, step, schedule, lambda, stall, stopping, projection, density, and objective combination are byte-unchanged.

## Paper basis and previous behavior

- Eq. (2) is implemented by the pure `netHpwl`/`exactHpwl` extrema scans using `LObject::cx/cy + LPin offset` because Level objects store lower-left coordinates and Level pins store center-relative offsets.
- Eq. (3) uses one weight-1 edge for degree 2 and the three weight-1/2 undirected edges for degree 3.
- Eqs. (6)-(8) use the directly constructed `2p-3` active edges for degree greater than 3, each weighted `1/(p-1)`.
- Eq. (13) uses sign away from a tie and seeded `cos(theta)` at ties, selected from previous pin coordinates.

Previously, Paper-L1 used nested `i,j` loops for every degree, so degree `p` visited `p(p-1)/2` pairs. Its `sx` helper returned zero whenever `|difference| <= EPS`, including exact ties. The sole old wirelength test checked HPWL equality, vector size, and fixed gradients, but did not independently compute Paper-L1 value, count active edges, test previous-position ties, or exercise high-degree scaling. Extrema mode distributed gradients over sets of object IDs near extrema, which also deduplicated multiple pins on one object. Tests confirmed the tie defect before the fix: the new two-pin tie test failed because the old gradient was zero. No function-value mismatch was demonstrated on valid Stage 1 inputs; the defect was complexity, missing observability, and incorrect/under-tested subgradient behavior.

## Implementation

- `WireEval` now reports exact HPWL (`hpwl`), independently accumulated `paper_l1_value`, x/y and total effective-edge counts, tie count, gradients, and maximum per-net L1/HPWL difference.
- `netHpwl` and `exactHpwl` are pure, finite-checked O(p) extrema evaluations. They do not own or touch evaluator state/RNG.
- Degree 2 and 3 have explicit constant-size edge construction. Degree greater than 3 has two single loops: min representative to every other pin, then max representative to every non-representative pin. It allocates no pair matrix or all-pairs list.
- Each axis independently selects extrema. The stable min representative is the lowest net-local pin index at the minimum; max is the highest index at the maximum. When all coordinates tie they remain distinct. The paper does not specify representative tie-breaking; this deterministic engineering rule does not alter the function value.
- Per-axis, per-net, and total independently accumulated Paper-L1 values are checked against extrema HPWL with `1e-12 * max(1, |HPWL|)` tolerance.
- `WirelengthEvaluator` stores previous x/y coordinates per net-local pin, never pair matrices. A level-index or topology-size change resets history. `reset(seed)` resets history and RNG.
- Stateful evaluations update previous coordinates and advance the RNG only for history-directed ties. `evaluateWirelength`, `exactHpwl`, and evaluator calls with `update_state=false` neither update history nor advance the RNG; no-history ties use `theta=0`, hence cosine 1.
- Fixed pins remain in value/edge construction, but gradient writes are suppressed for their objects. Multiple same-object pins are never deduplicated.

## Tie behavior

- Tie tolerance: `1e-14 * max(1, |a|, |b|)`, smaller than the repository `EPS`; it is used only for Eq. (13)/extrema tie decisions.
- Previous positive: theta uniform on `[0, pi/3]`, cosine in `[0.5, 1]`.
- Previous negative: theta uniform on `[2pi/3, pi]`, cosine in `[-1, -0.5]`.
- No history or previous tie: theta 0, cosine 1.
- Evaluator RNG: `std::mt19937_64`, explicitly seeded; no `random_device` or global mutable state.
- Optimizer evaluator seed is exposed as `OptimizeConfig::wire_seed` (default 1). Direct evaluator and optimizer-config tests can replay any chosen seed. The CLI Config seed is not wired to this new field because `app/main.cpp` was explicitly forbidden in Stage 2; that CLI plumbing remains a configuration-interface risk, not a mathematical/RNG reproducibility defect in the evaluator.
- Theta intervals are paper-defined. Relative tolerance, state reset topology signature, and stable representative selection are deterministic engineering details not specified by the paper.

## Complexity and performance

| Degree | Old pair visits per axis | New effective edges per axis | Expected |
|---:|---:|---:|---:|
| 2 | 1 | 1 | 1 |
| 3 | 3 | 3 | 3 |
| 4 | 6 | 5 | 5 |
| 5 | 10 | 7 | 7 |
| 1000 | 499500 | 1997 | 1997 |

The final degree-greater-than-3 path has no nested pin loop and is O(p) per axis. Exact HPWL is another O(p) scan. No p-by-p matrix or complete pair vector is allocated. The degree-1000 test observes exactly 1997 edges on each axis.

On adaptec1 level 0, Stage 1 Release `wirelengthSubgradient` took 0.152984 s, while the Stage 2 Release validated/stateless evaluation took 0.390565 s. Stage 1 visited 7,519,729 all-pairs entries; Stage 2 constructed 1,226,028 edges per axis (2,452,056 reported total), found 570,996 axis-edge ties, and had maximum per-net value difference `5.45697e-11`. The asymptotic edge count improved, but this benchmark timing is slower because Stage 2 additionally accumulates and validates independent values, tracks edge/tie diagnostics, and creates state-compatible coordinate storage. Whole adaptec1 smoke program runtime changed from 29.4083 s to 29.8402 s (+1.47%); this is reported rather than misrepresented as a speedup.

## Tests

- `test_hpwl_two_pin`: horizontal 3, vertical 2, diagonal/offset/fixed 12, tied 0; weight-1 edge counts and fixed gradients.
- `test_hpwl_three_pin`: exact/Paper-L1 9 with independent x/y extrema; repeated extrema value 7; three edges per axis.
- `test_hpwl_high_degree`: degrees 4, 5, 10, and 1000; exact/value equality, `2p-3` edge counts, repeat-call purity.
- `test_hpwl_pin_offsets`: three pins including two pins on the same object; exact/Paper-L1 16 and no pin deduplication.
- `test_hpwl_ties`: duplicate minima/maxima, permutation-invariant values, all-tied value 0, seven edges per axis and 14 ties for degree 5.
- `test_wire_subgradient`: center finite differences at a differentiable point, convex subgradient inequality at a tie, no-history cosine 1, previous-positive/negative intervals, antisymmetry, and fixed zero gradient.
- `test_wire_seed_reproducibility`: identical seed/sequence gives exactly identical gradients/ties; different seeds control a different sequence; report-only evaluation does not consume the history-directed sample.
- `test_wire_translation_invariance`: exact and Paper-L1 translation invariance plus zero all-movable gradient sums.
- `test_wire_descent_step`: direct small `-gradient` step without optimizer, with HPWL/OFR/fixed checks.
- `test_wirelength_properties`: fixed-seed random degrees 2-20, permutation and translation invariance, value equality, finite outputs, and fixed gradients.
- Existing `test_wirelength` remains enabled and passing.

## Build results

- Debug configure PASS: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- Debug build PASS: `cmake --build build --parallel`.
- CTest PASS: `ctest --test-dir build --output-on-failure`, 34/34.
- Release configure PASS: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release`.
- Release build PASS: `cmake --build build-release --parallel`, 89/89 actions on a clean Release tree.

## Formula-level results

| Case | Exact HPWL | Paper L1 | Difference | Edges/axis | Status |
|---|---:|---:|---:|---:|---|
| degree 2 diagonal | 12 | 12 | 0 | 1 | PASS |
| degree 3 | 9 | 9 | 0 | 3 | PASS |
| degree 4 | 6 | 6 | <=1e-12 | 5 | PASS |
| degree 5 | 8 | 8 | <=1e-12 | 7 | PASS |
| all tied degree 5 | 0 | 0 | 0 | 7 | PASS |
| multiple extrema | 9 | 9 | 0 | 5 | PASS |
| degree 1000 | 1998 | 1998 | <=1e-10 | 1997 | PASS |

## Controlled descent

| Metric | Before | After | Change |
|---|---:|---:|---:|
| HPWL | 30 | 29.7 | -0.3 |
| Paper OFR | 0 | 0 | 0 |
| Fixed displacement | 0 | 0 | 0 |

## Tiny regression

| Metric | Stage 1 | Stage 2 | Change |
|---|---:|---:|---:|
| Initial exact HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Final exact HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Final-PL Paper L1 | N/A | 1.2649588807450003 | matches reloaded exact |
| Paper OFR | 0.45579454177240702 | 0.45579454177240702 | 0 |
| Density penalty | 0.0073069534475406683 | 0.0073069534475406683 | 0 |
| Objective first row | 2.0302992740225911 | 2.4129694706612121 | +0.382670196638621 |
| Program runtime | 0.0469232 s | 0.0429317 s | -0.0039915 s |
| Final-PL tie count | N/A | 2 | diagnostic |
| Max per-net L1/HPWL difference | N/A | 0 | PASS |

The objective change is caused by the corrected tie gradients: wire-gradient L1 changes from 4 to 6, so the still-unmodified Eq. (16) lambda initialization changes. Final placement/HPWL/OFR/penalty remain unchanged; no optimizer rule was altered.

## adaptec1 smoke regression

| Metric | Stage 1 | Stage 2 | Change |
|---|---:|---:|---:|
| Initial exact HPWL | 1083429706.9169238 | 1083429706.916924 | +2.38419e-7 (2.20e-16 relative) |
| Final exact HPWL | 1083429706.9169238 | 1083429706.916924 | +2.38419e-7 (2.20e-16 relative) |
| Final-PL Paper L1 | N/A | 1083429706.9169238 | equals reloaded exact |
| Paper OFR | 0.12948416214601757 | 0.12948416214601757 | 0 |
| Density penalty | 3511339631728.3125 | 3511339631728.3125 | 0 |
| Objective first row | 1206033742.9984334 | 1205963568.4617953 | -70174.5366380 |
| Program runtime | 29.4083 s | 29.8402 s | +0.4319 s |
| Final-PL tie count | N/A | 6430 | diagnostic |
| Max per-net L1/HPWL difference | N/A | 2.14641e-10 | PASS |

The 2.20e-16 relative HPWL change is one aggregate floating-point rounding unit from the rewritten pure extrema initialization/accumulation and is far below the `1e-12` acceptance limit. Final PL is byte-identical to Stage 1. Corrected tie/active-edge gradients change wire-gradient L1 from 616714.98746979923 to 616362 and therefore change lambda/objective; density metrics do not change.

## Output and invariants

- Stage 2 and same-seed replay have byte-identical `final.pl`, `summary.json`, `hierarchy.json`, and `interlevel_hpwl.json`; `history.csv` excluding `elapsed_sec` is identical.
- Stage 1 and Stage 2 `final.pl`, hierarchy, and interlevel files are byte-identical. Summary/history legitimately differ in gradient-derived lambda/objective and one HPWL rounding unit.
- No NaN/Inf token occurs in output/history/logs; both output consistency objects are true.
- Since adaptec1 final PL is byte-identical to Stage 1, every fixed and movable output coordinate is unchanged in the smoke regression; fixed gradient tests independently prove zero fixed gradient.
- Every LPin is traversed, including multiple same-object pins. Exact HPWL is pure and repeatable. Report-only calls do not advance state/RNG. Same evaluator seed and sequence replay exactly.
- Values are translation invariant; all-movable gradients sum to zero; final PL reload HPWL equals summary/output-consistency HPWL within its configured tolerance.

## Remaining risks

- Density/OFR corrections are Stage 3 work and remain untouched.
- Optimizer first-evaluation stall is not fixed; PR, step, lambda conditions, and stopping remain unaudited until Stage 5.
- CLI `--seed` is not yet connected to `OptimizeConfig::wire_seed` because application changes were forbidden in Stage 2; the evaluator/config API itself is fully seeded and reproducible.
- Coarse pin preservation and internal nets are not fixed; multilevel HPWL continuity remains unverified.
- Legalization and detailed placement remain unavailable.
- Extrema representative tie-breaking and relative tie tolerance are paper-underspecified deterministic details.

## Commit

Commit message: `fix(wirelength): reproduce paper HPWL and l1 subgradient`. The final SHA is reported in the completion response because a commit cannot contain itself.

## Gate decision

**PASS — Stage 2 completed. Stop and wait for user approval before Stage 3.**
