# Full-text tuning follow-up

This candidate follows performance submission `67940fd92e9d579cd8cc75994976beb3792f9cee` on `sofanzz/fts-tuning-verified`.
The platform reported `fts_mean_improvement=-0.06796856197326077` and `fts_raw_score=0`.
Passing execution did not establish a performance improvement; a negative category-weighted improvement is clamped to zero.

## Changes and code-path evidence

- An omitted TOKENIZE parser configuration used to allocate a JSON tree, populate defaults, serialize it and parse it again in the parser helper. The empty configuration now uses the same constructor defaults directly. Explicit options still use the existing validation path.
- Word aggregation used a lookup followed by an overwriting insert. The hash table's overwrite implementation allocates a replacement node and frees the old one. `set_or_update` performs one lookup and updates the existing pair. The initial count remains one and subsequent counts add the supplied frequency, preserving previous behavior. The most recent collation-equal spelling is retained, as before.
- The document/query-local word maps and per-parser IK chain map use the existing no-lock hash specialization. Optimizer and both retrieval implementations use the shared word-map typedef, avoiding incompatible map types at `segment` calls.
- The built-in stopword set is populated during global parser initialization and is read-only until shutdown; its lookups use the no-lock specialization. Shared mutable dictionaries are not changed.

These changes reduce per-token work in indexing, TOKENIZE and search-token preparation. They do not remove posting-list reads or change ranking, token boundaries, collation comparison, stopword rules, input data or scoring.

## Validation status

This implementation has now been compiled and tested as part of the unified candidate. See [the measured follow-up](fts_verified_tuning.md) for correctness coverage, three full benchmark comparisons, and the outstanding gap to the full-score target.
