# Stage 5 optimizer gate audit

## Status

Stage 5 was entered only far enough to test the first-evaluation stall correction and parameter wiring. The candidate optimizer patch was **not retained** because it failed the mandatory tiny regression gate. No Stage 5 optimizer algorithm change is part of this commit.

## Candidate result

With the first objective evaluation excluded from the stall count, the prescribed tiny run performed four iterations instead of stopping before displacement. Relative to the Stage 4 baseline:

| Metric | Stage 4 | Candidate | Change |
|---|---:|---:|---:|
| Final HPWL | 1.2649588807453496 | 1.3047285713582482 | +3.1432% |
| Paper OFR | 0.45579454177240702 | 0.44879334033796431 | -1.5359% |
| Density penalty | 0.0073069534475406683 | 0.0071149837391969615 | -2.6270% |

The HPWL regression exceeds the 0.1% module gate. Adding an unreferenced heuristic to hide this tradeoff is prohibited, so Stage 5 must remain incomplete pending a focused optimizer audit.

## Repository cleanup

Generated `build/`, `build-release/`, `output/`, logs, Ninja metadata, libraries, object files, and test executables were removed from the working tree. Test **source** and committed fixtures are retained because they are required for reproducibility and CTest; they are not generated binaries. `.gitignore` now covers alternate `build-*` trees and generated CTest/Ninja metadata in addition to the existing build/output/binary patterns.

## Verification before cleanup

The unchanged Stage 4 code plus candidate-under-test compiled in Debug and Release, and CTest passed 61/61. After recording the failed regression, the candidate optimizer source changes were reverted. Generated build/output directories were then deleted so they cannot be accidentally uploaded.

## Gate

**FAIL — Stage 5 optimizer work is incomplete. Do not proceed to Stage 6.**
