# Current reproduction audit

## Baseline at `a42d888`

The pre-fix density evaluator subtracted fixed overlap from capacity, clamped the
result to `EPS`, and divided overflow by movable object area. Consequently a bin
whose fixed overlap alone exceeded capacity lost part of that overflow. The
penalty was the sum of squared positive *movable-capacity* overflow. Fixed
objects had no gradient, but did affect the reduced capacity.

Wirelength value was exact HPWL. Two- and three-pin gradients used the paper's
pair weights, while nets above degree three used an extrema-only subgradient.
Coordinate ties returned zero. Density overlap boundaries returned a fixed
half derivative rather than the randomized cosine subgradient.

The optimizer already used raw `W + lambda P`, standard (unclipped) PR beta,
the Equation (17) step schedule, per-stage best restoration, multiple stages by
default, and the paper-sized default `nmax`. However history incorrectly stored
OFR in `total_overflow`, and the summary hard-coded output consistency.

Clustering was batch best-choice with a degree cap, high-degree hub/chain
approximation, area-ratio score modifier, and geometric fallback pairing. It
was not Figure 2's dynamic global maximum-PQ process. Coarse/fine HPWL paired
nets by vector position rather than original net identity.

The multilevel driver performed coarsest quadratic initialization, optimization,
macro shifting on non-finest levels, and declustering from the current coarse
coordinates. It did not re-evaluate reported metrics after macro shifting and
did not implement a genuine output round trip. White-space allocation,
look-ahead legalization, NTUplace3 legalization, and detailed placement cannot
be reproduced from the available paper alone.

## Equation (18) repair

Paper mode now directly computes `D_b = movable_overlap + fixed_overlap`,
`overflow_b = max(0, D_b - target_density * bin_area)`, `P = sum overflow_b^2`,
and `paper_ofr = sum overflow_b / sum_all_object(width * height)`. The
denominator deliberately uses full object areas, including the part of a fixed
object outside the core; `total_clipped_area` separately records overlap with
the core. A level with non-positive total object area is an error.

## Classification

* **Paper-specified:** raw objective, Equation (18), fixed objects in density,
  movable-only position gradients, PR formula, step schedule, and lambda factors.
* **Paper-underspecified convention:** floating comparisons use `EPS`; complete
  object area is retained when fixed geometry extends outside the core.
* **Unavailable external components:** white-space allocation details,
  look-ahead legalization, NTUplace3 legalization, and detailed placement.
