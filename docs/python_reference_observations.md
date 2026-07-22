# Python reference observations

- `parse_pl` assigns `orientation` twice in the same branch; this appears redundant but is behavior-neutral and is not changed in the reference.
- Whitespace allocation code is present but intentionally disabled by the corrected main flow.
