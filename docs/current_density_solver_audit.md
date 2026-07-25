# Pre-fix density and solver audit

This audit records commit `6107b37` before the paper-mode corrections in this
change.

* Density used `max(0, movable - max(EPS, target * bin_area - fixed))`; its
  penalty was the sum of squared overflow. Penalty and reporting could use two
  independently configured target densities.
* Both OFR numerators summed the corresponding bin overflow, but both divided
  by movable-object area only. Fixed overlap reduced available capacity and
  therefore affected overflow, but the `max(EPS, ...)` capacity formulation did
  not preserve all fixed-only overflow when fixed area exceeded capacity.
* Initial lambda was the movable-only wire-gradient L1 norm divided by the
  movable-only density-gradient L1 norm; a non-positive density norm threw.
* The optimizer had an outer penalty-stage loop, stage-local lambda and raw
  `HPWL + lambda * penalty` best snapshots. The adaptec1 script nevertheless
  forced one stage. A stage restored its raw-objective best; a non-improving OFR
  restored the previous accepted snapshot.
* `nmax == 0` selected `max(1, min(ceil(0.001 * movable_count), 100))`, while the
  script overrode it with 1,000,000.
* History sourced HPWL, penalty, gradients and OFR from the current evaluations,
  but incorrectly wrote the dimensionless reporting OFR to `total_overflow`;
  `max_bin_overflow` and `projection_count` remained default placeholders.
* `summary.json` emitted a hard-coded zero solver/database error and
  `consistent: true`. Its top-level initial values came from the coarsest
  optimization result rather than the original benchmark level.
* `final.pl` was written after copying fine-level coordinates into the database,
  but was not parsed back or independently re-evaluated.
