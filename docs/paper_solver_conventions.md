# Paper solver conventions

The paper objective is implemented as `W + lambda*P`, where `P` is the sum of
squared positive bin overflow in area units.  Initial lambda is
`||g_W||_1/||g_P||_1` over movable objects.  A zero density-gradient norm is a
hard diagnostic error rather than an arbitrary fallback.

Polak--Ribiere is used without PR+, clipping, periodic restart, or a descent
test.  A zero previous norm restarts with steepest descent solely as a numeric
guard.  The displacement is `alpha*d`, with
`alpha=s*bin_width/||d||_2`; `s` is 0.2 and is multiplied by 2/3 every 100
iterations, with a floor of 0.06.

The integer no-improvement limit uses `ceil(0.001*n)`, capped at 100 and floored
at one for small tests.  A 10,000-iteration and 20-stage default are safety
limits.  The `1e-12*max(1,|value|)` comparisons, restoring the most recent
strictly OFR-improving stage, and core projection are engineering conventions
where the publication does not fully specify floating-point or boundary
policy.  Projection is performed after an update and is reported separately;
it is not folded into the search direction.
