# Full-text tuning: measured unified candidate

This report describes the previously tested v3 candidate, commit
`1705c0a5524ca6819a3b28e31065f9c78ef3287a`. The subsequent, untested changes
are documented separately in [the v4 submission note](fts_followup_v4.md).

## Result and scoring limitation

The previous platform submission `bb38260de42f156e94c9dd9e0eae92f82de58f1f`
completed successfully but scored zero (`fts_mean_improvement=-0.0557350931`).
This follow-up has been built and measured locally. It is **not a verified
full-score solution**: the three-category improvement target is 50%, while the
median-of-three same-machine comparison below is approximately 12.1%.
The actual platform baseline is unavailable. The much slower baseline included
in the public repository cannot establish the score on the current platform.

## Changes

1. For positive natural-language membership predicates, omit BM25 statistics and
   scoring when no expression observes the score. Eligibility is deliberately
   limited to a single base-table SELECT without lookup, LIMIT, ORDER BY,
   subqueries, set operations, boosts or another score consumer. Numeric score
   predicates, score projections, Boolean mode and ranked queries retain the
   existing path. Ordinary membership scans continue using document-at-a-time
   merging; an experimentally slower term-at-a-time variant was rejected.
2. For supported full-text auxiliary-index backfill keys, enable the existing
   order-preserving encoded sort. Other index DDL retains its previous policy.
   Key types and collations are checked before selecting it. The existing
   encoder supports VARCHAR through its variable-string encoder with CHAR
   padding disabled.
3. In the non-LOB utf8mb4_general_ci datum comparator, byte-identical non-NULL
   strings compare equal without decoding every character. All other inputs
   retain the original collation comparison.
4. English normalization uses an ASCII fast path for the two supported UTF-8
   collations. Already lowercase tokens reuse the input stream's valid buffer;
   uppercase ASCII is folded into an owned buffer. Non-ASCII and other
   collations retain the existing implementation.
5. IK no longer constructs the unused stop-word dictionary on each parse.
   Its stop-word filtering was already disabled, so emitted tokens are unchanged.
6. Retain the earlier default-parser-property, document-local map and
   single-lookup word aggregation improvements.

CPU samples identified repeated string comparison in FTS backfill, and
dictionary/cache and normalization work in TOKENIZE. No benchmark input, SQL,
timing, score calculation or baseline file was changed. No query-result cache
or test-data-specific shortcut is introduced.

## Local benchmark

WSL Ubuntu 22.04; isolated fresh instances; CPU count 2, memory 3 GiB;
same release build configuration for both executables. Official
`tools/benchmark/fts_large_bench.sh`: 20,000 rows, batch 500, 3,000 tokenization
rounds, 200 query rounds, 3 samples, 30 warmups, full index creation.
Three independent before/after pairs were run with order alternated. The
asynchronous AI package startup was awaited outside benchmark timing in both
versions. No CPU profiler ran during the reported timing measurements.

The comparison executable contains the earlier FTS implementation corresponding
to `67940fd9`; this is not a replay of the inaccessible platform baseline.
Binary SHA-256:

- Before: `e9396a4ee68ae7669129e2a090da1137ea164102e5eef895d2f7b9d8691f69d4`
- After: `8736ab5fb9e89b1cc7538c3dba674464b001e131c4e3a4abefd448f41f8e6a26`

The following are medians across the three runs, used only to summarize noise:

| Metric | Before | After |
|---|---:|---:|
| Build IK all (s) | 7.985 | 7.072 |
| Build IK content (s) | 6.045 | 5.438 |
| Build BENG (s) | 3.334 | 2.831 |
| TOKENIZE IK (ms) | 0.1665 | 0.1470 |
| TOKENIZE BENG (ms) | 0.1217 | 0.1105 |
| Query CN (ms) | 1.7047 | 1.3546 |
| Query BENG (ms) | 2.5447 | 1.9843 |
| Query mixed (ms) | 1.9035 | 1.5236 |
| Query LIMIT (ms) | 2.2105 | 2.3738 |

Category improvements from these medians: build 12.19%, tokenization 10.46%,
query 13.78%; equal-category mean 12.14%. LIMIT remains a regression/noise concern
and retains relevance calculation. Individual rounds and their weighted
improvements are preserved in `fts_local_comparison.json`; no run is discarded.
All benchmark hit counts remain 8001 / 11000 / 7332 / 20.

## Correctness and integration

The final executable passes the official TOKENIZE and simple-query suites and
four focused FTS suites: hot-path semantics, membership/score consumers,
collation/sorting and populated-table index backfill. Baseline-generated
reference results cover empty/NULL/stop-word input, duplicate tokens, prepared
statement reuse, over 256 search terms, updates/deletes, rollback, heap tables,
ranking, Boolean mode, forced encoded sorting and binary primary keys.
765 complete TOKENIZE outputs match the old executable byte-for-byte across five
parsers and three collations; four concurrent sessions also match.

The official simple-query reference changes only two EXPLAIN descriptions:
COUNT and DISTINCT-constant membership plans no longer calculate relevance.
All query-result rows remain unchanged. The initial unified harness still had a
stale staged copy of that reference; it was synchronized before the rerun.

This candidate is based on the unified feature branch and preserves its AI
startup fix, AI function enhancement, AI columns and file SQL implementation.
The submission tree is checked against the locally compiled source tree.
Platform correctness and score still require manual evaluation.

The unified regression rerun passes AI prompt/function enhancement, AI column
DDL/transaction visibility/DML/worker/retry/vector-index cases, CSV/JSONL/Parquet
and file streaming. Additional checks pass for file aggregation/joins/rescans,
concurrent file modification rejection, date ordering, retry-state persistence,
process-restart recovery and rejection of a late response after primary-key reuse.
