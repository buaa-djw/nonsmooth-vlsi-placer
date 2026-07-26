# Stage 3 exact density report

## Provenance and scope

- Algorithm baseline: `0078a9e4266d23b77c08f7c05f1697b5b34cc532`.
- Stage 0: `835ce1089175f94e93f910961f725c5e100a00ae`.
- Stage 1: `8753b66f90e539143eff0248dff65653c8a0aa29`.
- Stage 2 and Stage 3 parent: `6a2152d20d24898f609ea5582c1890bfb2874ef8`.
- Branch: `codex/paper-exact-reproduction`.
- Changes are limited to Density API/implementation/tests, legacy density-option validation, final/optimization density construction and consistency state in `app/main.cpp`, and this report. Wirelength, optimizer mathematics, multilevel, postprocess, database, I/O, Reporter, Python reference, and benchmarks are unchanged.

## Paper basis and previous behavior

- Eqs. (9)-(10): exact x/y interval intersection and rectangular overlap area are implemented by `exactOverlap1D` and `exactRectangleOverlap`.
- Eq. (11): each bin density is the sum of fixed and movable overlap, compared directly with `target_density * bin_area`.
- Eq. (12): the density penalty is the unnormalized sum of squared positive overflow; its subgradient coefficient is `2 * positive_overflow`.
- Eq. (18): OFR is total positive overflow divided by complete fixed-plus-movable object area.
- The density subgradient follows the paper's overlap-subgradient construction: x derivative is overlap-x subgradient times y overlap, and conversely for y.

Previously, exact overlap values, fixed-plus-movable density, direct capacity, squared penalty, and the complete-area OFR denominator were already correct on positive-overlap, nonempty Stage 2 cases. However, candidate upper bounds subtracted `EPS`, and the caller discarded `overlap <= 0`, so a touching movable object could not contribute a valid subgradient to a bin already overflowed by other objects. The pre-fix boundary test failed with zero gradient instead of positive inward gradient. Empty levels threw instead of returning finite zeros. The constructor accepted penalty/report densities but ignored report density. Main could construct optimization grids from legacy penalty/OFR values while final evaluation used target density, and its `db_level` final-consistency evaluation referenced the original rather than live final level. Existing tests checked several correct fixed/OFR numbers but expected empty evaluation to throw and did not expose per-bin arrays, conservation, target-density cases, boundary stencils, finite differences, deterministic properties, or optimization/final/reload configuration equality.

## Implementation

- `DensityConfig` contains exactly region, x/y bin counts, and one target density. The primary constructor validates positive region/bin geometry and `target_density in (0,1]`.
- Public exact overlap helpers expose overlap length, deterministic translation subgradient, and area for formula tests.
- Candidate ranges are derived from each object's closed rectangle against local bin indices. An object edge exactly on an internal boundary includes both adjacent bins; ranges are clamped and never scan all bins globally.
- Positive overlap alone contributes density area. Zero-area candidates are counted as touching and may retain a nonzero movable derivative stencil, but never add area.
- Fixed and movable overlaps enter one `bin_density_area`; fixed capacity is never subtracted or clipped. Fixed objects contribute full area to the OFR denominator but never receive gradient writes.
- `DensityEval` exposes full/movable/fixed/clipped area, total bin overlap, per-bin density/capacity/raw overflow/positive overflow/square, total overflow/penalty/OFR, maxima, overflow count, and candidate/positive/touching/gradient counts.
- Empty levels return penalty/OFR/areas zero, with finite per-bin raw overflow and no gradients.
- Evaluation is const and contains no RNG/cache/state mutation, so repeated calls are bitwise deterministic for the same Level/config.

## Boundary subgradient

For strict positive overlap, deterministic interval derivatives are 0 for containment, +1 for left entry, -1 for right exit, and 0 when separated. At external contact, the paper supplies a subdifferential but not a unique endpoint selection. Stage 3 chooses the deterministic inward one-sided endpoint: object-high equals bin-low gives +1; object-low equals bin-high gives -1. Contact overlap remains zero. A contact stencil contributes to penalty gradient only if the touched bin's positive overflow is nonzero. Corner contact has zero first-order area derivative because the orthogonal overlap length is zero. The boundary equality tolerance is `1e-14 * max(1, |a|, |b|)`; this selection is marked paper-underspecified and introduces no smoothing/random displacement.

## Target-density semantics

- `Config::target_density` is the sole run-time density used to construct optimization, final-live-level, and reloaded-final grids.
- Legacy `--penalty-density` and `--ofr-density` remain source/script compatible only when exactly equal to target density; a different value produces an explicit deprecation/error message. No `--report-density` option exists.
- The legacy five-value DensityGrid constructor likewise throws when penalty and report values differ; it delegates to the single-target implementation when equal.
- Final consistency now evaluates `levels.front()` (the live final state) and the reloaded PL with the same `DensityGrid`. Coordinate comparison likewise uses the live level, eliminating the original/final state mismatch without changing optimization rules.

## Formula-level results

| Case | Bin density | Capacity | Overflow | Penalty | OFR | Status |
|---|---:|---:|---:|---:|---:|---|
| under capacity | 40 | 50 | 0 | 0 | 0 | PASS |
| at capacity | 90 | 90 | 0 | 0 | 0 | PASS |
| movable overflow | 90 | 80 | 10 | 100 | 1/9 | PASS |
| fixed overflow | 64 | 50 | 14 | 196 | 14/64 | PASS |
| mixed overflow | 60 | 50 | 10 | 100 | 0.1 | PASS |
| partially outside core | 130 clipped | 50 | 80 | 6400 | 80/180 | PASS |
| empty level | 0 | 100 | 0 | 0 | 0 | PASS |

The partially-outside case has full object area 180, clipped/bin overlap 130, demonstrating that Eq. (18) uses full area rather than clipped or movable area.

## Subgradient results

- Differentiable fixture analytic x gradient: `-38`; centered finite difference: `-38.000000003535206`; absolute error `3.53521e-9`.
- Boundary fixture: bin already overflowed by 7.5, touching object's inward x gradient is 30; the corresponding under-capacity fixture gradient is zero.
- Vertical contact analog also returns 30; corner-contact area and first-order area gradient are zero.
- Fixed gradients are exactly zero in fixed-only, mixed, finite-difference, and descent fixtures.
- Boundary fixture visits 4 local candidates: 2 positive-overlap and 2 touching pairs, with 1 nonzero overflow-gradient contribution. No candidate is duplicated.

## Tests

- `test_density_overlap`: containment in both directions, left/right partial overlap and derivatives, separated/contact intervals, corner overlap, and non-square rectangles/bins.
- `test_density_conservation`: inside area 12 plus partially outside clipped area 4 equals total bin overlap 16; full object area remains 28.
- `test_density_fixed`: fixed-only density 64, capacity 50, overflow 14, penalty 196, OFR 14/64, zero gradient.
- `test_density_penalty_equation12`: two bins with densities 60/40, capacity 50, raw overflows 10/-10, total penalty 100.
- `test_density_ofr_equation18`: fixed area 80, movable full area 100, clipped overlap 130, overflow 80, penalty 6400, OFR 80/180.
- `test_density_target_density`: exact capacities/overflow/penalty/OFR for target 1.0, 0.9, 0.8, and 0.5.
- `test_density_boundary`: horizontal/vertical/corner contacts, overflowed and under-capacity bins, touching diagnostics, and no area duplication.
- `test_density_subgradient`: centered finite difference, contact subgradient interval, and fixed zero gradient.
- `test_density_descent_step`: direct density-only step without optimizer; penalty/OFR/HPWL/fixed/region checks.
- `test_density_empty`: empty, fixed-only, no-overflow, and 1x1-bin finite behavior.
- `test_density_properties`: fixed-seed random rectangles/grids, exact core-clipped conservation, nonnegative metrics, overflow counts, finite diagnostics, and repeat equality.
- `test_density_configuration_consistency`: identical optimization/final/reload values, legacy constructor rejection, and mismatched CLI rejection.
- Existing `test_density` remains and is strengthened to expect finite empty values.

## Build results

- Debug configure PASS: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- Debug build PASS: `cmake --build build --parallel`.
- CTest PASS: `ctest --test-dir build --output-on-failure`, 46/46.
- Release configure PASS: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release`.
- Release build PASS: `cmake --build build-release --parallel`, 113/113 actions on a clean Release tree.

## Controlled density descent

| Metric | Before | After | Change |
|---|---:|---:|---:|
| Density penalty | 90.25 | 88.811776 | -1.438224 |
| Paper OFR | 0.3958333333333333 | 0.3926666666666667 | -0.0031666666666666 |
| HPWL (no nets) | 0 | 0 | 0 |
| Fixed displacement | 0 | 0 | 0 |

The movable object remains inside the region. The direct step uses analytic density gradient `-38` and does not call or modify NonsmoothOptimizer.

## Tiny regression

| Metric | Stage 2 | Stage 3 | Change |
|---|---:|---:|---:|
| Initial HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Final HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Paper OFR | 0.45579454177240702 | 0.45579454177240702 | 0 |
| Density penalty | 0.0073069534475406683 | 0.0073069534475406683 | 0 |
| Objective first row | 2.4129694706612121 | 2.4129694706612121 | 0 |
| Total object area | N/A | 6 | diagnostic |
| Total clipped area (reloaded PL) | N/A | 6.0000000000001315 | diagnostic |
| Overflow bins | 1200 | 1224 | +24 |
| Program runtime | 0.0429317 s | 0.05021 s | +0.0072783 s |

The only non-timing history difference is overflow-bin count. Stage 2 counted only overflow greater than repository `EPS`; Eq. (12) defines positive overflow as strictly greater than zero, so Stage 3 correctly counts 24 additional tiny-positive bins. Their penalty/OFR/objective contributions were already included and all numeric metrics are unchanged.

## adaptec1 smoke regression

| Metric | Stage 2 | Stage 3 | Change |
|---|---:|---:|---:|
| Initial HPWL | 1083429706.916924 | 1083429706.916924 | 0 |
| Final HPWL | 1083429706.916924 | 1083429706.916924 | 0 |
| Paper OFR | 0.12948416214601757 | 0.12948416214601757 | 0 |
| Density penalty | 3511339631728.3125 | 3511339631728.3125 | 0 |
| Objective first row | 1205963568.4617953 | 1205963568.4617953 | 0 |
| Total object area | N/A | 101380284 | diagnostic |
| Total clipped area | N/A | 86450364 | diagnostic |
| Overflow bins | 71 | 71 | 0 |
| Program runtime | 29.8402 s | 29.38 s | -0.4602 s |

There are no non-timing numeric regressions. Final PL, summary, hierarchy, and interlevel files are byte-identical to Stage 2.

## Performance

| Dataset | Objects | Bins | Candidate pairs | Positive pairs | Touching pairs | Gradient contributions | Overflow bins | Release evaluation |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| tiny final | 3 | 22500 | 3648 | 3648 | 0 | 334 | 1224 | 0.003524 s |
| adaptec1 final | 211447 | 256 | 220013 | 220013 | 0 | 12052 | 71 | 0.048808 s |

Stage 2 adaptec1 Release density evaluation was 0.051195 s; Stage 3 is 0.048808 s. Candidate enumeration uses local x/y ranges; there is no object-times-all-bins loop, GPU, OpenMP, or global parallel refactor. Touching count is zero on these final placements because no object happens to lie exactly on a bin boundary; the dedicated boundary fixture proves those candidates are retained.

## Output and invariants

- Fixed objects contribute to density/overflow/penalty/OFR numerator and complete-area denominator; their gradients are zero.
- Capacity is always `target_density * bin_area`; no fixed subtraction or clipping occurs.
- All scalar/vector outputs are finite; empty evaluation is finite.
- Repeated Density evaluation is exactly deterministic and evaluation-order independent.
- Stage 3 and same-seed replay have byte-identical final PL, summary, hierarchy, and interlevel files; history excluding elapsed is identical.
- Stage 2/Stage 3 final PL and summary are byte-identical for both regressions. Tiny history differs only in the corrected positive-overflow bin count.
- Live-final, reloaded-final, and optimizer density use the same target density. Both output-consistency objects are true.
- Every Level object is included; there is no benchmark-specific branch, coordinate mutation, RNG, or stale cache.

## Remaining risks

- Full combined objective-gradient integration is Stage 4 work.
- Lambda0 is not formally audited; optimizer stall, step, PR, lambda updates, and stopping remain Stage 5 work.
- CLI wire seed remains unconnected as documented in Stage 2.
- Clustering/coarse-pin/internal-net preservation and multilevel continuity remain unresolved.
- Legalization and detailed placement are unavailable.
- Stage 2 wire evaluator constant overhead remains.
- Boundary subgradient endpoint and `1e-14` equality tolerance are deterministic paper-underspecified choices.

## Commit

Commit message: `fix(density): reproduce exact penalty and paper OFR`. The resulting SHA is reported in the completion response because a commit cannot contain itself.

## Gate decision

**PASS — Stage 3 completed. Stop and wait for user approval before Stage 4.**
