# Paper solver conventions

The penalty loop uses a deterministic accepted-stage snapshot. After each stage
its raw-objective best position is restored and re-evaluated. If its OFR fails
to improve the latest accepted OFR by `1e-12 * max(1, abs(previous_ofr))`, the
latest accepted coordinates are restored and optimization stops with
`ofr_not_improved`. This restoration is the deterministic engineering
realization of the paper's instruction to stop when OFR no longer decreases.
