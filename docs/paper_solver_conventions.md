# Paper solver conventions

The paper objective is implemented as `W + lambda * P`, where `P` is the sum of
squared positive bin overflow in area units. Initial lambda is
`||g_W||_1 / ||g_P||_1` over movable objects. A zero density-gradient norm is a
hard diagnostic error rather than an arbitrary fallback.

Polak--Ribiere is used without PR+, clipping, periodic restart, or a descent
test. A zero previous norm restarts with steepest descent solely as a numeric
guard. The displacement is `alpha * d`, with
`alpha = s * bin_width / ||d||_2`; `s` is 0.2 and is multiplied by 2/3 every
100 iterations, with a floor of 0.06.

The integer no-improvement limit uses `ceil(0.001 * n)`, capped at 100 and
floored at one for small tests. The iteration and stage limits are safety
limits. Projection is performed after an update and is reported separately; it
is not folded into the search direction.

## Deterministic accepted-stage convention

After each penalty stage, the solver restores that stage's best raw-objective
snapshot and independently re-evaluates it. A stage is accepted only when its
OFR improves the most recently accepted OFR by more than
`1e-12 * max(1, abs(previous_ofr))`. If it does not, the solver restores the
most recent accepted coordinates and stops with `ofr_not_improved`. This is the
deterministic engineering realization of the paper's instruction to stop when
OFR no longer decreases; it also prevents a rejected stage from leaking worse
coordinates into the next level or `final.pl`.
