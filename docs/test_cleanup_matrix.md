# Test cleanup matrix

| Existing test | Module | Covered behavior | Duplicate with | Action | Destination |
|---|---|---|---|---|---|
| `tests/unit/test_bookshelf_aux.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_bookshelf_counts.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_bookshelf_gzip.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_bookshelf_invalid.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_bookshelf_reader.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_clustering.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_clustering.cpp` |
| `tests/unit/test_config.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_config.cpp` |
| `tests/unit/test_database_invariants.cpp` | PlacementDB | Existing module-specific coverage | none | keep | `tests/unit/test_database_invariants.cpp` |
| `tests/unit/test_declustering.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_declustering.cpp` |
| `tests/unit/test_density.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | keep | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_boundary.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_configuration_consistency.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_conservation.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_descent_step.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_empty.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_fixed.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_ofr_equation18.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_overlap.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_penalty_equation12.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_properties.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_subgradient.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_density_target_density.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_fixed_terminal.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_hpwl_high_degree.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_hpwl_pin_offsets.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_hpwl_three_pin.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_hpwl_ties.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_hpwl_two_pin.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_macro_shifting.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_macro_shifting.cpp` |
| `tests/unit/test_native_end_to_end_tiny.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_native_end_to_end_tiny.cpp` |
| `tests/unit/test_objective_call_count.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_combined.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_controlled_descent.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_density_only.cpp` | Density | Existing numeric/formula cases retained verbatim | test_density.cpp | merge | `tests/unit/test_density.cpp` |
| `tests/unit/test_objective_finite_difference.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_fixed.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_invalid_lambda.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_lambda_scaling.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_opposing_gradients.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_properties.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_seed_replay.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_state_semantics.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_tie_and_boundary.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_value.cpp` | Objective | Existing numeric/formula cases retained verbatim | test_objective.cpp | merge | `tests/unit/test_objective.cpp` |
| `tests/unit/test_objective_wire_only.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_optimizer_helpers.cpp` | Optimizer | Existing numeric/formula cases retained verbatim | test_optimizer.cpp | merge | `tests/unit/test_optimizer.cpp` |
| `tests/unit/test_paper_optimizer_math.cpp` | Optimizer | Existing numeric/formula cases retained verbatim | test_optimizer.cpp | merge | `tests/unit/test_optimizer.cpp` |
| `tests/unit/test_pin_offsets.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_placement_db.cpp` | PlacementDB | Existing module-specific coverage | none | keep | `tests/unit/test_placement_db.cpp` |
| `tests/unit/test_projection.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_projection.cpp` |
| `tests/unit/test_python_random.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_python_random.cpp` |
| `tests/unit/test_quadratic_initializer.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_quadratic_initializer.cpp` |
| `tests/unit/test_scl_subrows.cpp` | Bookshelf | Existing numeric/formula cases retained verbatim | test_bookshelf.cpp | merge | `tests/unit/test_bookshelf.cpp` |
| `tests/unit/test_spatial_hash.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_spatial_hash.cpp` |
| `tests/unit/test_whitespace_allocator.cpp` | Existing later-stage/utility | Existing module-specific coverage | none | keep | `tests/unit/test_whitespace_allocator.cpp` |
| `tests/unit/test_wire_descent_step.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_wire_seed_reproducibility.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_wire_subgradient.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_wire_translation_invariance.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_wirelength.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | keep | `tests/unit/test_wirelength.cpp` |
| `tests/unit/test_wirelength_properties.cpp` | Wirelength | Existing numeric/formula cases retained verbatim | test_wirelength.cpp | merge | `tests/unit/test_wirelength.cpp` |
