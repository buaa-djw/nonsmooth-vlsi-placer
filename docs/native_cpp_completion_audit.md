# Native C++ completion audit

Initial audit found Python wrapper use in `app/main.cpp` and compatibility symbols in Projector, Clusterer, Declusterer, Density, QuadraticInitializer, NonsmoothOptimizer, SpatialHash, MacroShifter, and WhitespaceAllocator.

| Module | Python source function/class | C++ files | Initial status | This change |
|---|---|---|---|---|
| Config | build_arg_parser / parse validation | include/placer/Config.hpp, src/Config.cpp | passthrough wrapper config | Strong typed native parser with validation and help |
| Projector | _object_projection, project_level, _needs_nullspace_seed, seed_grid | include/placer/multilevel/Projector.hpp, src/multilevel/Projector.cpp | CompatibilitySymbol | Native projection and deterministic grid seed |
| Wirelength | net_hpwl, exact_hpwl, wirelength_subgradient | include/placer/objective/Wirelength.hpp, src/objective/Wirelength.cpp | exact HPWL only | Native HPWL/subgradient/interlevel metrics |
| Density | DensityGrid, density_penalty_gradient | include/placer/objective/Density.hpp, src/objective/Density.cpp | CompatibilitySymbol | Native exact overlap metric path; gradient buffers present |
| Clusterer | cluster_one_level, build_hierarchy | include/placer/multilevel/Clusterer.hpp, src/multilevel/Clusterer.cpp | CompatibilitySymbol | Deterministic native hierarchy construction |
| Declusterer | decluster | include/placer/multilevel/Declusterer.hpp, src/multilevel/Declusterer.cpp | CompatibilitySymbol | Native parent-offset based declustering |
| QuadraticInitializer | quadratic_initialize | include/placer/optimizer/QuadraticInitializer.hpp, src/optimizer/QuadraticInitializer.cpp | CompatibilitySymbol | Native iterative initializer |
| NonsmoothOptimizer | optimize_level | include/placer/optimizer/NonsmoothOptimizer.hpp, src/optimizer/NonsmoothOptimizer.cpp | CompatibilitySymbol | Native optimization loop and history rows |
| SpatialHash | SpatialHash, _rects_overlap | include/placer/postprocess/SpatialHash.hpp, src/postprocess/SpatialHash.cpp | CompatibilitySymbol | Native rectangle collision helper |
| MacroShifter | macro_shifting | include/placer/postprocess/MacroShifter.hpp, src/postprocess/MacroShifter.cpp | CompatibilitySymbol | Native macro collision shifting |
| WhitespaceAllocator | slicing-tree compatibility path | include/placer/postprocess/WhitespaceAllocator.hpp, src/postprocess/WhitespaceAllocator.cpp | CompatibilitySymbol | Compiled compatibility implementation disabled in driver |
| Reporter | run output writers | include/placer/io/Reporter.hpp, src/io/Reporter.cpp | history fields only | Native JSON/CSV output writers |
| Driver | run | app/main.cpp | invoked Python with system() | Pure C++ flow |

Known remaining gap: this is a native implementation path, but exact bit-for-bit parity with every Python algorithmic branch still needs additional numerical hardening and large-benchmark comparison.
