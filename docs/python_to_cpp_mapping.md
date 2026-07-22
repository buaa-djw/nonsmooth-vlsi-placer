# Python to C++ mapping

Fully read reference file: `reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py` (1928 lines).

## Classes/data structures
- `Cell`
- `Pin`
- `Net`
- `Row`
- `PlacementDB`
- `LObject`
- `LPin`
- `LNet`
- `Level`
- `DensityGrid`
- `DensityEval`
- `WireEval`
- `OptimizeConfig`
- `SpatialHash`
- `SliceNode`

## Functions
- `_open_text` line 143
- `_strip_comment` line 149
- `_tokens` line 153
- `_resolve_aux_reference` line 157
- `parse_aux` line 168
- `parse_nodes` line 186
- `parse_pl` line 201
- `parse_scl` line 224
- `parse_nets` line 271
- `load_bookshelf` line 309
- `_median_or` line 391
- `build_level0` line 395
- `_object_projection` line 416
- `project_level` line 435
- `_needs_nullspace_seed` line 441
- `seed_grid` line 454
- `_group_connectivity` line 478
- `_fallback_pairing` line 522
- `_shelf_pack` line 541
- `_compact_child_offsets` line 573
- `cluster_one_level` line 612
- `build_hierarchy` line 770
- `quadratic_initialize` line 797
- `_edge_derivative` line 926
- `_overlap_1d_and_derivative` line 934
- `density_penalty_gradient` line 963
- `_pair_abs_gradient` line 1031
- `wirelength_subgradient` line 1041
- `_net_hpwl` line 1083
- `exact_hpwl` line 1091
- `interlevel_hpwl_consistency` line 1095
- `_rms` line 1119
- `_dot` line 1125
- `optimize_level` line 1148
- `_rects_overlap` line 1339
- `macro_shifting` line 1374
- `_rect_overlap_area` line 1450
- `_build_slice_tree` line 1455
- `_compute_slice_demand` line 1480
- `_affine_map` line 1498
- `_allocate_slice` line 1505
- `whitespace_allocation` line 1528
- `decluster` line 1565
- `write_level_pl` line 1644
- `write_final_pl` line 1652
- `_history_fields` line 1665
- `run` line 1674
- `build_arg_parser` line 1830
- `main` line 1887

## CLI arguments/defaults
- `'aux'` default `(see source)`
- `'--out'` default `'output/nonsmooth_single'`
- `'--wirelength-mode'` default `'paper_l1'`
- `'--target-density'` default `1.0`
- `'--penalty-density'` default `None`
- `'--ofr-density'` default `None`
- `'--bins'` default `None`
- `'--current'` default `150`
- `'--coarsen-ratio'` default `5.0`
- `'--max-levels'` default `16`
- `'--cluster-degree-cap'` default `256`
- `'--quadratic-init'` default `True`
- `'--quadratic-iterations'` default `200`
- `'--quadratic-damping'` default `0.75`
- `'--quadratic-anchor'` default `0.0001`
- `'--quadratic-tolerance'` default `0.001`
- `'--iterations'` default `None`
- `'--iterations-per-stage'` default `100`
- `'--penalty-stages'` default `4`
- `'--density-only'` default `False`
- `'--lambda0'` default `0.0`
- `'--density-gradient-ratio'` default `1.0`
- `'--lambda-growth-high'` default `2.2`
- `'--lambda-growth-mid'` default `1.9`
- `'--lambda-growth-low'` default `1.6`
- `'--s0'` default `0.0`
- `'--s-floor'` default `0.0`
- `'--step-decay'` default `200.0`
- `'--target-ofr'` default `0.0`
- `'--report-every'` default `10`
- `'--nmax'` default `1000000`
- `'--hpwl-continuity-tol'` default `1e-08`
- `'--macro-shifting'` default `True`
- `'--macro-search-rings'` default `30`
- `'--macro-gap'` default `0.0`
- `'--whitespace-allocation'` default `False`
- `'--wsa-leaf-size'` default `64`
- `'--wsa-min-fraction'` default `0.05`
- `'--seed'` default `1`

## Output files
`hierarchy.json`, `coarsest_before_quadratic.pl`, `coarsest_after_quadratic.pl`, `history.csv`, `interlevel_hpwl.json`, `summary.json`, `run_info.json`, `level_<index>_final.pl`, `final.pl`.

## Algorithm stages
Parse args; load Bookshelf (nodes, pl, scl, nets); region; build level0; hierarchy; coarsest quadratic/seed; coarse-to-fine decluster, HPWL check, bins, projection, density grid, nonsmooth optimize, macro shifting, per-level PL; final DB/PL; JSON/CSV reports.

## Constants/exceptions/boundaries
`EPS=1.0e-12`; duplicate cell, missing aux references/files, unknown cell in net, premature net EOF, invalid hierarchy mapping, object/envelope impossible projection, child packing impossible, CLI validation errors. Boundary handling includes row-derived region preference, `.pl.gz` fallback, colon tokenization, fixed/terminal semantics, density edge derivative 0.5 on equal boundary, and WSA disabled in main flow.
