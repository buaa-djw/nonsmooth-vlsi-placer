# Stage 4 combined objective report

## 1. Module
Paper Combined Objective and Complete Subgradient Integration.

## 2. Provenance
- Algorithm baseline: `0078a9e4266d23b77c08f7c05f1697b5b34cc532`.
- Stage 0: `835ce1089175f94e93f910961f725c5e100a00ae`.
- Stage 1: `8753b66f90e539143eff0248dff65653c8a0aa29`.
- Stage 2: `6a2152d20d24898f609ea5582c1890bfb2874ef8`.
- Stage 3 and Stage 4 parent: `4cbb6e2a173ec51d3745dc3ab881e6ecc9bcc605`.
- Branch: `codex/paper-exact-reproduction`; the worktree was clean before Stage 4.
- No Git remote is configured (`git remote -v` is empty); `git ls-remote origin ...` failed because `origin` does not exist. Stage 3 was therefore truthfully treated as a local-only commit. Stage 4 cannot be pushed from this checkout without inventing a remote.

## 3. Scope compliance
Changed files are the new Objective API/implementation, the minimal optimizer integration, CMake source registration, objective-only unit tests, and this report. Wirelength and Density source/header mathematics are unchanged. Optimizer PR, direction, alpha, step schedule, nmax, stall, best restore, lambda update, OFR stopping, projection, and iteration limits are unchanged. Config, app, multilevel, postprocess, database, I/O, Python reference, and benchmarks are unchanged.

## 4. Paper basis
The evaluator implements `F = W + lambda P` and `g = gW + lambda gP` directly. `exact_hpwl` remains a report/final metric; independently computed `paper_l1_value` is W; Stage 3 `quadratic_penalty` is P; `paper_ofr` is report/stopping-only and never enters F. There is no normalization, clipping, balancing, hidden coefficient, or OFR term.

## 5. Previous behavior and call-graph audit
Before Stage 4, `optimizeLevel` directly assembled values and gradients in `src/optimizer/NonsmoothOptimizer.cpp`:
1. Objective used `w.hpwl` (exact HPWL), not the independently named `paper_l1_value`; Stage 2 made them numerically equivalent, but the role was not explicit.
2. Combined movable gradient was algebraically `w.g + lambda*d.g`.
3. No density normalization, gradient-ratio use, clipping, component normalization, or hidden coefficient was present in this function.
4. Stage 2/3 component evaluators independently returned zero fixed gradients; the optimizer iterated only movable IDs, but did not audit both components.
5. A stateful wire evaluation occurred once in each iteration, plus initialization, stage initialization, restored stage-best evaluation, and final evaluation.
6. `report_every` only printed the already evaluated values and did not itself call the evaluator.
7. The lambda0 input-gradient evaluation was stateful and advanced wire history/RNG; Stage 4 retains this control-flow behavior and records it as Stage 5 risk.
8. Restored stage-best evaluation was stateful and advanced history/RNG even though only values were required.
9. Final reporting evaluation was stateful.
10. History objective was `exact_hpwl + lambda*penalty`. Because Stage 2 proved exact HPWL equals Paper-L1 within tolerance, numbers were correct, but it did not explicitly consume the Paper-L1 field.

The initial objective tests did not compile before Objective API implementation; the new state/call-count tests then exposed the absence of explicit value/state modes and auditable call counters.

## 6. Changes
- `ObjectiveEvaluation` separately exposes HPWL, Paper-L1, penalty, OFR, lambda, objective, component and combined gradients, component norms, ties, overflow diagnostics.
- `ObjectiveEvaluator` validates finite nonnegative lambda and calculates the two paper identities without scaling.
- `ValueOnly` evaluates a copy of the stateful wire evaluator, so RNG and previous positions in the owned evaluator cannot change; it returns values without retaining large gradient vectors.
- `StatefulGradient` commits the wire evaluator exactly once, evaluates Density once, validates vector/object identity and finiteness, audits fixed component gradients, and combines each x/y component.
- Lightweight counters expose value/stateful/density evaluations and wire commits for tests.
- Optimizer initialization remains one stateful evaluation for current lambda0 behavior. Stage initialization, restored-stage-best evaluation, and final reporting are now value-only. Each optimizer iteration performs exactly one stateful combined evaluation. No optimizer update equation changed.

## 7. Formula-level results
| Case | W | P | Lambda | Expected F | Actual F | Difference |
|---|---:|---:|---:|---:|---:|---:|
| wire only | 14 | 0 | 7 | 14 | 14 | 0 |
| density only | 0 | 576 | 2 | 1152 | 1152 | 0 |
| combined | 13 | 576 | 0.25 | 157 | 157 | 0 |
| lambda zero | 13 | 576 | 0 | 13 | 13 | 0 |
| opposing gradients | 13 | 576 | 1 | 589 | 589 | 0 |

Lambda checks at 2.5 and 10 produced 1453 and 5773 respectively, exactly following the affine identity.

## 8. Gradient component results
Representative combined fixture, x direction:
| Object | gW_x | gP_x | Lambda | Expected g_x | Actual g_x |
|---|---:|---:|---:|---:|---:|
| movable `a` | -2 | 192 | 0.25 | 46 | 46 |
| fixed `fixed` | 0 | 0 | 0.25 | 0 | 0 |

For this fixture all reported y components are zero: `gW_y=0`, `gP_y=0`, expected/actual combined y=0. Separate finite-difference and tie tests exercise nonzero/tied y behavior.

## 9. Evaluation-state audit
- Per optimizer iteration: one stateful combined call, one wire commit, one Density evaluation.
- Report-only/value-only: zero wire commits and no mutation/RNG consumption; four previews followed by one stateful evaluation produced counters `value=4`, `stateful=1`, `wire commits=1`, `density=5`.
- Previous pin positions update only through StatefulGradient.
- Lambda0 preview behavior: initialization remains stateful once; formal lambda0/control-flow repair remains Stage 5.
- Stage initialization and restored-best reevaluation: ValueOnly, no commit.
- Final evaluation/reporting: ValueOnly, no commit.
- Report printing consumes the current iteration result and performs no evaluation.

## 10. Tests
Fifteen registered tests cover objective values at five lambdas; natural wire-only and density-only cases; component-by-component combined and opposing gradients; lambda linearity; fixed terminal/macro participation and zero gradients; simultaneous wire ties and density boundary contact; report/state sequence equivalence; seed replay; invalid lambda errors; differentiable multi-object finite differences; controlled fixed-alpha descent; call counts; and fixed-seed randomized properties. Stage 2/3 tests remain unchanged and passing.

## 11. Build results
- Debug configure PASS: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- Debug build PASS: `cmake --build build --parallel`.
- CTest PASS: `ctest --test-dir build --output-on-failure`, 61/61.
- Release configure PASS: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release`.
- Release build PASS: `cmake --build build-release --parallel`; clean Release build completed 144/144 actions, followed by a successful incremental verification.

## 12. Controlled combined descent
| Metric | Before | After | Change |
|---|---:|---:|---:|
| Objective | 18.759999999999998 | 18.759999360010237 | -0.000000639989761 |
| HPWL | 13 | 12.999984 | -0.000016 |
| Density penalty | 576 | 576.00153600102385 | +0.00153600102385 |
| Paper OFR | 0.5 | 0.50000066666666665 | +0.000000666666667 |
| Fixed displacement | 0 | 0 | 0 |

With fixed test-only `alpha=1e-4` and lambda `0.01`, objective and HPWL strictly improve; OFR degradation is about 0.000133%, below the 0.1% gate. No line search is implemented and all objects remain legal.

## 13. Tiny regression
| Metric | Stage 3 | Stage 4 | Change |
|---|---:|---:|---:|
| Initial HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Final HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Paper OFR | 0.45579454177240702 | 0.45579454177240702 | 0 |
| Density penalty | 0.0073069534475406683 | 0.0073069534475406683 | 0 |
| Objective first row | 2.4129694706612121 | 2.4129694706612121 | 0 |
| Stateful iteration calls | N/A | 2 total (1 per row) | diagnostic |
| Runtime | 0.05021 s | 0.057 s | +0.00679 s |

## 14. adaptec1 regression
| Metric | Stage 3 | Stage 4 | Change |
|---|---:|---:|---:|
| Initial HPWL | 1083429706.916924 | 1083429706.916924 | 0 |
| Final HPWL | 1083429706.916924 | 1083429706.916924 | 0 |
| Paper OFR | 0.12948416214601757 | 0.12948416214601757 | 0 |
| Density penalty | 3511339631728.3125 | 3511339631728.3125 | 0 |
| Objective first row | 1205963568.4617953 | 1205963568.4617953 | 0 |
| Stateful iteration calls | N/A | 1 | diagnostic |
| Runtime | 29.38 s | 29.959 s | +0.579 s |

All non-timing metrics are unchanged. Runtime variations are wall-clock noise on Debug smoke runs, not an algorithmic change.

## 15. History consistency
All 2 tiny rows and the 1 adaptec1 row were checked. Maximum absolute error for `raw_objective - (raw_hpwl + lambda*raw_density_penalty)` is exactly 0. Stage 2 guarantees raw HPWL/Paper-L1 equality at these points. No stale component value or report-only mutation was observed.

## 16. Invariants
Objective and component identities, lambda linearity, zero fixed component/combined gradients, pure value calls, no report RNG consumption, one commit per stateful evaluation, finite outputs, seed replay, final PL round trip, output consistency, and absence of benchmark-specific branches all pass. Same-seed tiny/adaptec1 replay produced byte-identical `final.pl`, `summary.json`, `hierarchy.json`, and `interlevel_hpwl.json`.

## 17. Remaining risks
Lambda0 has not been formally repaired; CLI wire seed remains unconnected; PR beta and step/s schedule remain unaudited; first-evaluation stall, stage stopping, and stage-best rules remain Stage 5 work; clustering/coarse pins and multilevel continuity remain unresolved; legalization and detailed placement remain unavailable.

## 18. Commit
Commit message: `fix(objective): integrate paper wirelength and density subgradients`. The resulting SHA is reported in the completion response because a commit cannot contain itself. Parent is Stage 3 `4cbb6e2a173ec51d3745dc3ab881e6ecc9bcc605`. No remote exists, so push status is `local-only; not pushed`; no force push was attempted.

## 19. Gate decision
**PASS — Stage 4 completed. Stop and wait for user approval before Stage 5.**
