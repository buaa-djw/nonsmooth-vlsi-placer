# Test cleanup report

## 1. Provenance
- Stage 5 algorithm commit: `1bd5a7e84c61b992d0b9173913c82192a3b0a200`.
- Cleanup parent: Stage 5 commit above.
- Branch: `codex/paper-exact-reproduction`.

## 2. Scope
Only tests, the Stage 1 text fixture reference, `.gitignore`, and cleanup documentation change. Production `src/`, `include/`, app, Config, benchmark, and Python reference are unchanged by cleanup.

## 3. Before/after inventory
| Category | Before | After |
|---|---:|---:|
| Test source files | 61 | 17 |
| Test executables | 61 | 17 |
| CTest entries | 61 | 17 |
| Support headers | 4 | 4 |
| Binary fixtures | 1 | 0 |
| Generated tracked files | 0 | 0 |

The five paper-reproduction modules retain 50 former executable test bodies as independently invoked functions inside five consolidated executables; 11 unrelated existing module tests remain separate.

## 4. Consolidation map
The exhaustive old-file-to-destination mapping is in `docs/test_cleanup_matrix.md`. Bookshelf cases moved to `test_bookshelf.cpp`; HPWL/wire cases to `test_wirelength.cpp`; density cases to `test_density.cpp`; objective cases to `test_objective.cpp`; optimizer cases to `test_optimizer.cpp`. Each former `main` remains a separately called function and retains its original assertions.

## 5. Coverage matrix
- Parser/PlacementDB: counts/adaptec integration, AUX, invalid inputs, gzip, fixed/orientation, offsets, subrows, and database invariants remain.
- Wirelength: degrees 0/1/2/3/4/5/1000, offsets, ties/history/seed, fixed gradients, translation, finite difference, nonsmooth rules, and descent remain.
- Density: overlap/conservation, fixed overflow, Eqs. 12/18, target density, boundary, gradient, empty/1x1/outside, configuration, descent, and properties remain.
- Objective: scalar/component identities, wire/density-only, opposition, lambda, fixed, tie/boundary, state/seed, finite difference, descent, and call counts remain.
- Optimizer: the existing formula helpers and integration assertions, including lambda0, raw PR, step schedule, lambda updates, nmax and first-evaluation stall, remain.

## 6. Binary/generated-file cleanup
The tracked `valid.nodes.gz` fixture was removed. The gzip test now creates a unique temporary directory, writes gzip content through zlib (not a system gzip command), parses it, and removes it. Build/output directories and binary suffixes are ignored; no generated file is tracked.

## 7. Build and test
Debug configure/build PASS. Consolidated CTest PASS 17/17. Release configure/build PASS.

## 8. Regression
| Metric | Before cleanup | After cleanup | Difference |
|---|---:|---:|---:|
| tiny HPWL | 1.3047285713582482 | 1.3047285713582482 | 0 |
| tiny OFR | 0.4487933403379643 | 0.4487933403379643 | 0 |
| tiny penalty | 0.0071149837391969615 | 0.0071149837391969615 | 0 |
| adaptec1 HPWL | 1083429706.916924 | 1083429706.916924 | 0 |
| adaptec1 OFR | 0.12948416214601757 | 0.12948416214601757 | 0 |
| adaptec1 penalty | 3511339631728.3125 | 3511339631728.3125 | 0 |

Both final PL files are byte-identical before/after cleanup.

## 9. Repository hygiene
No tracked build files, added binary fixtures, stale targets, or production modifications. Generated output remains ignored. `git diff --check` passes.

## 10. Commit
Commit message: `test(repo): consolidate paper reproduction tests`. SHA is reported after commit.

## 11. Gate
PASS — Test cleanup completed. Stop and wait for user approval before Stage 6.
