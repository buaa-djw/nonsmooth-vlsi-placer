# Current solver audit

The pre-fix C++ solver optimized the normalized diagnostic
`W/W_initial + lambda*P/P_initial`, initialized lambda from normalized RMS
gradients with a silent fallback of one, used PR+ plus a descent restart, and
used the rational displacement schedule `s0/(1+k/200)`.  Although a stage
loop existed, smoke scripts selected one stage.  Its per-stage best objective
was not accompanied by a coordinate snapshot; a separate global `(OFR,HPWL)`
snapshot was restored only after all stages.

Coordinates are owned by each hierarchy `Level` during optimization.  The
driver writes that `Level`, declusters from that same optimized coarse level,
and `writeFinalPl` synchronizes level zero into `PlacementDB`.  Thus the
observed unchanged coarse file was caused by restoring the global OFR/HPWL
snapshot, frequently the quadratic start, rather than by optimizing a copied
database.  Summary evaluation also used the restored level while history
described transient working coordinates.

Previously clustering remapped pins to coarse object IDs but retained duplicate
incidences and nets that became internal.  Coarse net counts could consequently
remain identical.  Coarse construction now sorts and uniquifies incidences and
omits nets with fewer than two distinct coarse objects.

The paper mode now uses raw `W + lambda*P`, raw gradients, an independent best
snapshot for every fixed-lambda stage, and restores the last stage whose OFR
strictly improves.  Output is produced only from the resulting authoritative
level-zero state.
