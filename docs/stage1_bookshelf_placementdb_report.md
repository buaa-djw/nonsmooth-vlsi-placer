# Stage 1 Bookshelf input and PlacementDB report

## Stage 0 provenance

- Algorithm baseline SHA: `0078a9e4266d23b77c08f7c05f1697b5b34cc532`.
- Stage 0 report commit and Stage 1 parent: `835ce1089175f94e93f910961f725c5e100a00ae`.
- Branch: `codex/paper-exact-reproduction`.
- Stage 0 changed only `baseline_report.md`; Stage 1 is a separate change set.

## Scope compliance

Stage 1 changes are limited to `PlacementDB`, `BookshelfReader`, `TextInput`, parser/database tests, the small `tests/data/stage1` fixture, and this required report. No application, multilevel, objective, optimizer, postprocess, writer, Python-reference, or benchmark file was modified. No placement algorithm was changed.

## Verified previous behavior

- `parseNodes`, `parsePl`, and `parseScl` caught `std::invalid_argument` and silently continued. Numeric suffixes, non-finite values, and most malformed records were not rejected consistently.
- Declared `NumNodes`, `NumTerminals`, `NumRows`, `NumNets`, and `NumPins` were skipped rather than retained or checked.
- Unknown PL cells were silently ignored. Unknown net pin cells did throw, but without file/line context.
- Node terminal detection used a substring match, so unknown markers containing `terminal` could be misclassified.
- SCL fields used `map[key]` defaults and one map entry per field, losing all but the last subrow in a CoreRow. All eight available ISPD 2005 SCL files were inspected: their CoreRows each have one subrow; a synthetic multi-subrow fixture was therefore added.
- Gzip whole-file reading already existed, but zlib read/close failures were not checked. AUX only fell back automatically from an uncompressed name to compressed `.pl`, not other required file types, and conflicting references overwrote one another.
- `PlacementDB::addPin` did not validate endpoint IDs or finite offsets, and no whole-database invariant checker existed.

The pre-fix `test_bookshelf_invalid` was run against a `NumNodes` mismatch and failed because no exception was thrown, establishing that the new test detects an existing defect.

## Changes

- `PlacementDB` now stores optional Bookshelf declared counts, validates dimensions/endpoints/offsets on insertion, and provides a complete database invariant check covering name mapping, terminal/fixed semantics, pin ownership, net identity, and row geometry.
- `BookshelfReader` now performs strict finite/full-token numeric conversion with path, line, field/object, and raw-line context. It validates all available declared counts and preserves degree-0/1 nets in the database.
- AUX parsing now supports multiline and reordered references, quoted paths, backslash separators, relative paths, and `.gz` variants for every required type. Missing, nonexistent, and conflicting references are rejected.
- PL parsing now rejects unknown/duplicate/missing cells and malformed coordinates, retains orientation, and implements exact `/FIXED` and `/FIXED_NI` handling while preserving `terminal => fixed`.
- SCL parsing validates all required fields and `End`, keeps every subrow as an independent `Row`, validates positive geometry/counts, and calculates `x_end = x_start + NumSites * Sitespacing`.
- NETS parsing validates declarations, degree, premature headers/EOF, cell references, directions, finite offsets, and stable names. No parser-stage net filtering was introduced.
- `TextInput` now reports gzip decompression and close errors. Whole-file reading remains intentionally unchanged to avoid an out-of-scope I/O redesign.

## Input-format validation

| Format | Declared counts | Actual counts | Malformed input | gzip | Status |
|---|---|---|---|---|---|
| AUX | N/A | Four required unique references | Missing/conflicting/nonexistent references rejected | All four suffix types recognized | PASS |
| NODES | `NumNodes`, `NumTerminals` optional and retained | Cells and exact terminal markers checked | Duplicate names, invalid/non-finite/non-positive dimensions and bad counts rejected | Tested with `.nodes.gz` | PASS |
| PL | N/A | Exactly one record per known cell | Unknown/duplicate/missing cell, bad coordinate/token rejected | Supported by common reader/AUX | PASS |
| SCL | `NumRows` optional and checked against CoreRows | Every subrow retained as a Row | Missing `End`/field, invalid geometry/count rejected | Supported by common reader/AUX | PASS |
| NETS | `NumNets`, `NumPins` optional and retained | Net degree, total nets, and total pins checked | Unknown cell, malformed offset/degree, premature header/EOF rejected | Supported by common reader/AUX | PASS |

## Database invariants and counts

| Dataset | Cells | Terminals | Fixed | Movable | Nets | Pins | CoreRows declared | Rows/subrows stored |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `tiny_basic` | 3 | 0 | 0 | 3 | 2 | 5 | 2 | 2 |
| Stage 1 synthetic fixture | 4 | 1 | 3 | 1 | 2 | 4 | 1 | 2 |
| adaptec1 | 211447 | 543 | 543 | 210904 | 221142 | 944053 | 890 | 890 |

The invariant tests establish `terminal => fixed`, exact cell-name map identity, valid pin cell/net IDs, exactly one net ownership per pin, matching `pin.net_id`, valid `net.pin_ids`, finite numeric data, and row endpoint consistency. The fixture also confirms that a degree-1 net remains in `PlacementDB`.

## Build and test results

- Debug configure PASS: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- Debug build PASS: `cmake --build build --parallel`.
- CTest PASS: `ctest --test-dir build --output-on-failure`, 24/24 tests.
- Release configure PASS: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release`.
- Release build PASS: `cmake --build build-release --parallel`, 69/69 Ninja actions.

## Tests

- `test_bookshelf_counts`: optional declared metadata and exact fixture counts; when present, adaptec1 integration checks 211447 cells, 543 fixed cells, 221142 nets, 944053 pins, and 890 rows. It prints an explicit skip if the benchmark is unavailable.
- `test_pin_offsets`: positive, negative, fractional, and zero offsets plus `O`, `I`, and `B` directions.
- `test_fixed_terminal`: terminal without a PL fixed marker, nonterminal `/FIXED`, nonterminal `/FIXED_NI`, movable cell, and four retained orientations.
- `test_bookshelf_gzip`: committed deterministic `.nodes.gz` fixture and AUX resolution.
- `test_bookshelf_aux`: multiline/reordered references, quoted path containing a space, backslash separator, and conflicting references.
- `test_bookshelf_invalid`: count mismatches, degree mismatch, premature EOF, duplicate node, unknown pin/PL cell, malformed/trailing numeric text, NaN, Inf, zero size, missing SCL field, and missing AUX reference; checks contextual message substrings.
- `test_scl_subrows`: two disjoint subrows in one CoreRow, independent row endpoints, and bounding region without inventing a stored row across the gap.
- `test_database_invariants`: all PlacementDB cross-reference, fixed/terminal, name-map, degree-1 preservation, and row geometry invariants.

## Baseline metric comparison

| Metric | Stage 0 tiny | Stage 1 tiny | Delta |
|---|---:|---:|---:|
| HPWL | 1.2649588807453496 | 1.2649588807453496 | 0 |
| Paper OFR | 0.45579454177240702 | 0.45579454177240702 | 0 |
| Density penalty | 0.0073069534475406683 | 0.0073069534475406683 | 0 |
| Objective first row | 2.0302992740225911 | 2.0302992740225911 | 0 |

| Metric | Stage 0 adaptec1 | Stage 1 adaptec1 | Delta |
|---|---:|---:|---:|
| HPWL | 1083429706.9169238 | 1083429706.9169238 | 0 |
| Paper OFR | 0.12948416214601757 | 0.12948416214601757 | 0 |
| Density penalty | 3511339631728.3125 | 3511339631728.3125 | 0 |
| Objective first row | 1206033742.9984334 | 1206033742.9984334 | 0 |

There are no numerical differences to explain. Both output consistency checks remain true.

## Artifact comparison

For tiny and adaptec1, Stage 0 and Stage 1 have byte-identical `final.pl`, `summary.json`, `hierarchy.json`, and `interlevel_hpwl.json`. Every `history.csv` field except `elapsed_sec` is identical; only expected timing values differ. `run_info.json` was excluded as specified.

## Stage 0 issue impact

- The strict declared-count checks confirm that the adaptec1 input database is complete at 221142 nets and 944053 pins.
- Level 0 remains at 219794 active nets and 942705 incidences. Because no Level or algorithm file changed, this difference is downstream filtering of nets with fewer than two pins, not parser loss. Its correctness remains a later-stage audit item.
- The C++ and Python parsers still differ: the immutable Python reference silently skips malformed nodes, malformed/unknown PL records, malformed net declarations/offsets, and malformed SCL values, and retains only one subrow per CoreRow. Stage 1 deliberately does not modify the reference file.

## Remaining risks

- `readTextFile()` still materializes each entire input file. This is a peak-memory risk for large ISPD inputs; a streaming parser can be considered later, but was intentionally excluded from this minimal correctness change.
- Level-0 single-pin-net filtering remains unmodified and has not been judged algorithmically correct in Stage 1.
- HPWL and OFR formula reproduction work has not begun.
- No multilevel run was used, so coarse/fine net preservation and HPWL continuity remain unverified.
- The parser validates but does not store SCL `Siteorient` and `Sitesymmetry`, because the permitted existing `Row` public data model has no such fields and placement algorithms do not currently consume them. They are required syntactically and cannot silently default.

## Commit

Stage 1 is committed separately with message `fix(io): validate Bookshelf input and PlacementDB invariants`. The resulting SHA is reported in the module completion response because a commit cannot contain its own SHA.

## Gate decision

**PASS — Stage 1 completed. Stop and wait for user approval before Stage 2.**
