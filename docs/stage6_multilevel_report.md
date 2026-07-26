# Stage 6: multilevel clustering, declustering, and HPWL diagnostics

## Scope and paper basis

This stage changes only hierarchy construction, hierarchy expansion, and interlevel diagnostics. It does not change exact HPWL/Paper-L1 mathematics, density, the objective, the nonsmooth optimizer, macro shifting, WSA, or legalization.

The implementation follows Zhu et al. (TCAD 2015): Fig. 2's modified best-choice clustering, Eq. (19)'s area-weighted clustering score, and Fig. 3's rule that cells/subclusters inherit the optimized parent location during declustering.

## Problems before this stage

* Connectivity discarded fixed and macro pins, reducing hyperedge degree and incorrectly making externally connected nets look internal.
* Packed child offsets affected cluster envelopes, coarse pin offsets, parent clamping, and declustered child locations.
* Coarse/fine HPWL diagnostics paired nets by vector index even though internal nets are filtered and ordering can change.
* Invalid and duplicate original net identities and invalid hierarchy mappings could pass silently.

## New clustering semantics

For every net, pins are deduplicated into distinct coarse endpoints. A movable standard cell maps to its active standard group; each fixed object and movable macro maps to its own singleton object endpoint. Every incident net increments `E_j` once for each candidate group endpoint. For `d` distinct endpoints, `D_jk` increments by `1/(d-1)` only when both endpoints are movable standard groups. Thus fixed objects and macros affect degree but never form merge pairs.

The score is Eq. (19) with `A(a)=1/a`. Areas and connectivity must be positive/finite. If `E-D <= EPS`, that term represents completely internalizable connectivity and has infinite (highest) priority; stable `(cluster_id_a, cluster_id_b)` ordering breaks equal scores. Connectivity and the queue are currently rebuilt after every merge.

Cluster area is exactly the sum of child physical areas. Width/height use the area and a child-area-weighted aspect ratio clamped to `[0.25,4]`; this does not place children. Paper-mode child offsets are all `(0,0)`, no artificial projection bounding box is installed, and fixed/macro singletons retain dimensions and coordinates.

Each original net has at most one zero-offset pin per coarse object. Pins are sorted by object ID and nets by `original_net_id`. Nets reduced to one endpoint are omitted as internalized, while their identity remains on the fine level. Invalid/duplicate IDs are errors. Transition validation checks ownership, singleton macro/fixed preservation, legal pins, finite geometry, and unique coarse IDs.

## New declustering semantics

Declustering never changes the optimized parent and ignores `child_offsets` and the region. Every movable child receives exactly the parent center; every fixed child remains bit-for-bit unchanged. Projection remains the caller's next step. `inherited_children` and `fixed_children_unchanged` describe the operation. Compatibility fields `shifted_parents`, `impossible_groups`, and `max_parent_shift` remain and are always zero.

## Interlevel HPWL diagnostics

Both levels are indexed exclusively by valid, unique `original_net_id`; there is no vector-index fallback. Ordered maps provide deterministic traversal. Diagnostics define:

* `paired_nets`: IDs in both levels;
* `fine_only_nets`: normally internalized nets, with HPWL accumulated in `internalized_fine_hpwl`;
* `coarse_only_nets`: anomalous coarse-only IDs, with HPWL accumulated in `coarse_only_hpwl`;
* `matched_coarse_hpwl` and `matched_fine_hpwl`: matched contributions;
* `sum_abs_net_delta` and `max_abs_net_delta`: matched absolute differences plus each unmatched net's absolute contribution.

The overall coarse/fine totals, delta, relative delta, and ratio remain diagnostics and are not required to be identical.

## Verification results

Debug configuration/build and all 18 CTest tests passed. Tests include hand-computable fixed endpoint and macro endpoint cases, stable merge selection, coarse-pin deduplication/zero offsets, internalized nets, strict center inheritance despite nonzero stored offsets, fixed-child preservation, reordered-ID pairing, fine/coarse-only contributions, and duplicate/invalid-ID failures.

The tiny Bookshelf fixture was forced to two hierarchy levels in `test_clustering`: 3 movable objects became 2, mappings repeated identically, all pin IDs were legal, and all geometry was finite. The normal tiny end-to-end test also passed. A separate forced-hierarchy CLI experiment constructed two levels but the existing optimizer rejected its one-bin coarsest state because the initial density-gradient L1 was zero; this optimizer behavior is outside this stage and is not hidden by changing its mathematics.

`adaptec1` is present. A Release smoke command loaded 211,447 cells, 221,142 nets, and 944,053 pins, but a 20-second resource limit expired during hierarchy construction before a level completed. It produced no numerical result and did not modify benchmark data.

No NaN or Inf occurred in successful tests. The synthetic fixed-child declustering test confirms fixed coordinates remain unchanged, parent shift is zero, and its diagnostic has no coarse-only net unless one is deliberately inserted by the ID-matching test.

## Performance and remaining risks

Distinct endpoint construction and ordered containers add validation/determinism overhead. More importantly, the current correctness-first Clusterer still rebuilds all connectivity and its priority queue after every merge; the adaptec1 timeout demonstrates that this is the dominant scalability risk.

Remaining out of scope: incremental/look-ahead clustering optimization, look-ahead legalization, WSA/full postprocessing reproduction, and final legalization/detailed placement. Results remain global placement results and are not directly comparable to the paper's final legalized table values.
