# Python/C++ Completeness Matrix
This matrix was generated after reading the full Python reference file. Status values intentionally use the requested vocabulary.
| Python symbol | Python line range | Python purpose | C++ header | C++ source | C++ symbol | Implementation status | Test file | Parity status |
|---|---:|---|---|---|---|---|---|---|
| `Cell` | 48-68 | class/dataclass from Python reference | `include/placer` | `src` | `Cell` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Cell.area` | 59-60 | method/property | `include/placer` | `src` | `Cell.area` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Cell.cx` | 63-64 | method/property | `include/placer` | `src` | `Cell.cx` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Cell.cy` | 67-68 | method/property | `include/placer` | `src` | `Cell.cy` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Pin` | 72-77 | class/dataclass from Python reference | `include/placer` | `src` | `Pin` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Net` | 81-83 | class/dataclass from Python reference | `include/placer` | `src` | `Net` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Row` | 87-94 | class/dataclass from Python reference | `include/placer` | `src` | `Row` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `PlacementDB` | 98-140 | class/dataclass from Python reference | `include/placer` | `src` | `PlacementDB` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `PlacementDB.add_cell` | 105-112 | method/property | `include/placer` | `src` | `PlacementDB.add_cell` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `PlacementDB.add_net` | 114-117 | method/property | `include/placer` | `src` | `PlacementDB.add_net` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `PlacementDB.add_pin` | 119-124 | method/property | `include/placer` | `src` | `PlacementDB.add_pin` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `PlacementDB.region` | 126-140 | method/property | `include/placer` | `src` | `PlacementDB.region` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_open_text` | 143-146 | function/helper/algorithm stage | `include/placer` | `src` | `_open_text` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_strip_comment` | 149-150 | function/helper/algorithm stage | `include/placer` | `src` | `_strip_comment` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_tokens` | 153-154 | function/helper/algorithm stage | `include/placer` | `src` | `_tokens` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_resolve_aux_reference` | 157-165 | function/helper/algorithm stage | `include/placer` | `src` | `_resolve_aux_reference` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `parse_aux` | 168-183 | function/helper/algorithm stage | `include/placer` | `src` | `parse_aux` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `parse_nodes` | 186-198 | function/helper/algorithm stage | `include/placer` | `src` | `parse_nodes` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `parse_pl` | 201-221 | function/helper/algorithm stage | `include/placer` | `src` | `parse_pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `parse_scl` | 224-268 | function/helper/algorithm stage | `include/placer` | `src` | `parse_scl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `parse_nets` | 271-306 | function/helper/algorithm stage | `include/placer` | `src` | `parse_nets` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `load_bookshelf` | 309-316 | function/helper/algorithm stage | `include/placer` | `src` | `load_bookshelf` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LObject` | 325-355 | class/dataclass from Python reference | `include/placer` | `src` | `LObject` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LObject.area` | 342-343 | method/property | `include/placer` | `src` | `LObject.area` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LObject.cx` | 346-347 | method/property | `include/placer` | `src` | `LObject.cx` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LObject.cy` | 350-351 | method/property | `include/placer` | `src` | `LObject.cy` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LObject.set_center` | 353-355 | method/property | `include/placer` | `src` | `LObject.set_center` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LPin` | 359-362 | class/dataclass from Python reference | `include/placer` | `src` | `LPin` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `LNet` | 366-368 | class/dataclass from Python reference | `include/placer` | `src` | `LNet` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Level` | 372-388 | class/dataclass from Python reference | `include/placer` | `src` | `Level` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Level.movable_ids` | 379-380 | method/property | `include/placer` | `src` | `Level.movable_ids` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Level.standard_movable_ids` | 383-384 | method/property | `include/placer` | `src` | `Level.standard_movable_ids` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `Level.macro_ids` | 387-388 | method/property | `include/placer` | `src` | `Level.macro_ids` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_median_or` | 391-392 | function/helper/algorithm stage | `include/placer` | `src` | `_median_or` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `build_level0` | 395-413 | function/helper/algorithm stage | `include/placer` | `src` | `build_level0` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_object_projection` | 416-432 | function/helper/algorithm stage | `include/placer` | `src` | `_object_projection` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `project_level` | 435-438 | function/helper/algorithm stage | `include/placer` | `src` | `project_level` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_needs_nullspace_seed` | 441-451 | function/helper/algorithm stage | `include/placer` | `src` | `_needs_nullspace_seed` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `seed_grid` | 454-470 | function/helper/algorithm stage | `include/placer` | `src` | `seed_grid` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_group_connectivity` | 478-519 | function/helper/algorithm stage | `include/placer` | `src` | `_group_connectivity` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_fallback_pairing` | 522-537 | function/helper/algorithm stage | `include/placer` | `src` | `_fallback_pairing` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_shelf_pack` | 541-570 | function/helper/algorithm stage | `include/placer` | `src` | `_shelf_pack` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_compact_child_offsets` | 573-609 | function/helper/algorithm stage | `include/placer` | `src` | `_compact_child_offsets` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `cluster_one_level` | 612-767 | function/helper/algorithm stage | `include/placer` | `src` | `cluster_one_level` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `build_hierarchy` | 770-789 | function/helper/algorithm stage | `include/placer` | `src` | `build_hierarchy` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `quadratic_initialize` | 797-856 | function/helper/algorithm stage | `include/placer` | `src` | `quadratic_initialize` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid` | 865-923 | class/dataclass from Python reference | `include/placer` | `src` | `DensityGrid` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid.__post_init__` | 881-888 | method/property | `include/placer` | `src` | `DensityGrid.__post_init__` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid.index` | 890-891 | method/property | `include/placer` | `src` | `DensityGrid.index` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid.build_fixed` | 893-901 | method/property | `include/placer` | `src` | `DensityGrid.build_fixed` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid.bounds_for_rect` | 903-908 | method/property | `include/placer` | `src` | `DensityGrid.bounds_for_rect` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityGrid.deposit` | 910-923 | method/property | `include/placer` | `src` | `DensityGrid.deposit` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_edge_derivative` | 926-931 | function/helper/algorithm stage | `include/placer` | `src` | `_edge_derivative` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_overlap_1d_and_derivative` | 934-948 | function/helper/algorithm stage | `include/placer` | `src` | `_overlap_1d_and_derivative` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `DensityEval` | 952-960 | class/dataclass from Python reference | `include/placer` | `src` | `DensityEval` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `density_penalty_gradient` | 963-1021 | function/helper/algorithm stage | `include/placer` | `src` | `density_penalty_gradient` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `WireEval` | 1025-1028 | class/dataclass from Python reference | `include/placer` | `src` | `WireEval` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_pair_abs_gradient` | 1031-1038 | function/helper/algorithm stage | `include/placer` | `src` | `_pair_abs_gradient` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `wirelength_subgradient` | 1041-1079 | function/helper/algorithm stage | `include/placer` | `src` | `wirelength_subgradient` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_net_hpwl` | 1083-1088 | function/helper/algorithm stage | `include/placer` | `src` | `_net_hpwl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `exact_hpwl` | 1091-1092 | function/helper/algorithm stage | `include/placer` | `src` | `exact_hpwl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `interlevel_hpwl_consistency` | 1095-1116 | function/helper/algorithm stage | `include/placer` | `src` | `interlevel_hpwl_consistency` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_rms` | 1119-1122 | function/helper/algorithm stage | `include/placer` | `src` | `_rms` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_dot` | 1125-1126 | function/helper/algorithm stage | `include/placer` | `src` | `_dot` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `OptimizeConfig` | 1130-1145 | class/dataclass from Python reference | `include/placer` | `src` | `OptimizeConfig` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `optimize_level` | 1148-1331 | function/helper/algorithm stage | `include/placer` | `src` | `optimize_level` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_rects_overlap` | 1339-1341 | function/helper/algorithm stage | `include/placer` | `src` | `_rects_overlap` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SpatialHash` | 1344-1371 | class/dataclass from Python reference | `include/placer` | `src` | `SpatialHash` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SpatialHash.__init__` | 1345-1347 | method/property | `include/placer` | `src` | `SpatialHash.__init__` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SpatialHash._keys` | 1349-1355 | method/property | `include/placer` | `src` | `SpatialHash._keys` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SpatialHash.add` | 1357-1359 | method/property | `include/placer` | `src` | `SpatialHash.add` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SpatialHash.collides` | 1361-1371 | method/property | `include/placer` | `src` | `SpatialHash.collides` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `macro_shifting` | 1374-1431 | function/helper/algorithm stage | `include/placer` | `src` | `macro_shifting` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `SliceNode` | 1440-1447 | class/dataclass from Python reference | `include/placer` | `src` | `SliceNode` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_rect_overlap_area` | 1450-1452 | function/helper/algorithm stage | `include/placer` | `src` | `_rect_overlap_area` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_build_slice_tree` | 1455-1477 | function/helper/algorithm stage | `include/placer` | `src` | `_build_slice_tree` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_compute_slice_demand` | 1480-1495 | function/helper/algorithm stage | `include/placer` | `src` | `_compute_slice_demand` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_affine_map` | 1498-1502 | function/helper/algorithm stage | `include/placer` | `src` | `_affine_map` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_allocate_slice` | 1505-1525 | function/helper/algorithm stage | `include/placer` | `src` | `_allocate_slice` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `whitespace_allocation` | 1528-1557 | function/helper/algorithm stage | `include/placer` | `src` | `whitespace_allocation` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `decluster` | 1565-1636 | function/helper/algorithm stage | `include/placer` | `src` | `decluster` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `write_level_pl` | 1644-1649 | function/helper/algorithm stage | `include/placer` | `src` | `write_level_pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `write_final_pl` | 1652-1662 | function/helper/algorithm stage | `include/placer` | `src` | `write_final_pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `_history_fields` | 1665-1671 | function/helper/algorithm stage | `include/placer` | `src` | `_history_fields` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `run` | 1674-1827 | function/helper/algorithm stage | `include/placer` | `src` | `run` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `build_arg_parser` | 1830-1884 | function/helper/algorithm stage | `include/placer` | `src` | `build_arg_parser` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `main` | 1887-1924 | function/helper/algorithm stage | `include/placer` | `src` | `main` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI aux` | 1832 | argparse option | `include/placer` | `src` | `Config::aux` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --out` | 1833 | argparse option | `include/placer` | `src` | `Config::--out` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --wirelength-mode` | 1834 | argparse option | `include/placer` | `src` | `Config::--wirelength-mode` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --target-density` | 1835 | argparse option | `include/placer` | `src` | `Config::--target-density` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --penalty-density` | 1837 | argparse option | `include/placer` | `src` | `Config::--penalty-density` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --ofr-density` | 1839 | argparse option | `include/placer` | `src` | `Config::--ofr-density` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --bins` | 1841 | argparse option | `include/placer` | `src` | `Config::--bins` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --current` | 1842 | argparse option | `include/placer` | `src` | `Config::--current` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --coarsen-ratio` | 1844 | argparse option | `include/placer` | `src` | `Config::--coarsen-ratio` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --max-levels` | 1845 | argparse option | `include/placer` | `src` | `Config::--max-levels` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --cluster-degree-cap` | 1846 | argparse option | `include/placer` | `src` | `Config::--cluster-degree-cap` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --quadratic-init` | 1848 | argparse option | `include/placer` | `src` | `Config::--quadratic-init` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --quadratic-iterations` | 1849 | argparse option | `include/placer` | `src` | `Config::--quadratic-iterations` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --quadratic-damping` | 1850 | argparse option | `include/placer` | `src` | `Config::--quadratic-damping` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --quadratic-anchor` | 1851 | argparse option | `include/placer` | `src` | `Config::--quadratic-anchor` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --quadratic-tolerance` | 1852 | argparse option | `include/placer` | `src` | `Config::--quadratic-tolerance` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --iterations` | 1854 | argparse option | `include/placer` | `src` | `Config::--iterations` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --iterations-per-stage` | 1856 | argparse option | `include/placer` | `src` | `Config::--iterations-per-stage` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --penalty-stages` | 1857 | argparse option | `include/placer` | `src` | `Config::--penalty-stages` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --density-only` | 1858 | argparse option | `include/placer` | `src` | `Config::--density-only` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --lambda0` | 1859 | argparse option | `include/placer` | `src` | `Config::--lambda0` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --density-gradient-ratio` | 1861 | argparse option | `include/placer` | `src` | `Config::--density-gradient-ratio` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --lambda-growth-high` | 1862 | argparse option | `include/placer` | `src` | `Config::--lambda-growth-high` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --lambda-growth-mid` | 1863 | argparse option | `include/placer` | `src` | `Config::--lambda-growth-mid` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --lambda-growth-low` | 1864 | argparse option | `include/placer` | `src` | `Config::--lambda-growth-low` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --s0` | 1865 | argparse option | `include/placer` | `src` | `Config::--s0` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --s-floor` | 1867 | argparse option | `include/placer` | `src` | `Config::--s-floor` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --step-decay` | 1868 | argparse option | `include/placer` | `src` | `Config::--step-decay` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --target-ofr` | 1869 | argparse option | `include/placer` | `src` | `Config::--target-ofr` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --report-every` | 1870 | argparse option | `include/placer` | `src` | `Config::--report-every` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --nmax` | 1871 | argparse option | `include/placer` | `src` | `Config::--nmax` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --hpwl-continuity-tol` | 1873 | argparse option | `include/placer` | `src` | `Config::--hpwl-continuity-tol` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --macro-shifting` | 1876 | argparse option | `include/placer` | `src` | `Config::--macro-shifting` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --macro-search-rings` | 1877 | argparse option | `include/placer` | `src` | `Config::--macro-search-rings` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --macro-gap` | 1878 | argparse option | `include/placer` | `src` | `Config::--macro-gap` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --whitespace-allocation` | 1879 | argparse option | `include/placer` | `src` | `Config::--whitespace-allocation` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --wsa-leaf-size` | 1881 | argparse option | `include/placer` | `src` | `Config::--wsa-leaf-size` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --wsa-min-fraction` | 1882 | argparse option | `include/placer` | `src` | `Config::--wsa-min-fraction` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `CLI --seed` | 1883 | argparse option | `include/placer` | `src` | `Config::--seed` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output hierarchy.json` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::hierarchy.json` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output coarsest_before_quadratic.pl` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::coarsest_before_quadratic.pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output coarsest_after_quadratic.pl` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::coarsest_after_quadratic.pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output history.csv` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::history.csv` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output interlevel_hpwl.json` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::interlevel_hpwl.json` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output summary.json` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::summary.json` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output run_info.json` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::run_info.json` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output level_<index>_final.pl` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::level_<index>_final.pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `output final.pl` | 1674-1827 | output artifact | `include/placer` | `src` | `Reporter::final.pl` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage load Bookshelf` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::load Bookshelf` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage build level0` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::build level0` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage build hierarchy` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::build hierarchy` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage quadratic initialize` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::quadratic initialize` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage coarse-to-fine optimize` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::coarse-to-fine optimize` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage decluster` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::decluster` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage macro shifting` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::macro shifting` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
| `stage write reports` | 1674-1827 | driver algorithm stage | `include/placer` | `src` | `driver::write reports` | IMPLEMENTED | `tests/unit` | IMPLEMENTED |
