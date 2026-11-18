# FTS v4: submission candidate, not locally tested

Parent: `1705c0a5524ca6819a3b28e31065f9c78ef3287a`.
The parent platform run reported mean improvement 0.03783262698981849 and
raw score 7.566525397963699. The user reported a displayed score of 3.783.
CN/BENG/mixed query times improved, while tokenization was almost unchanged
and IK content-index creation regressed in that run. One build measurement
does not establish which part of the build path caused the regression.

## Changes

- Prune token frequency and document length from the inverted-index access and
  output projections when relevance is not calculated. The earlier change
  disabled BM25 evaluation but still decoded these unused columns. Document IDs
  remain available to merging; ranked queries retain all scoring inputs. Code
  generation allows the absent document-length expression only for unscored scans.
- Give each parser-local DAT reader a one-character lookup cache. IK extends
  multiple active prefixes using the same character, previously repeating the
  immutable dictionary's character-to-code hash lookup. Both successful and
  missing codes can be reused for identical bytes within that reader. Every
  prefix transition and match decision still runs independently. No shared
  dictionary bytes, query results, token boundaries or dictionary versions change.
  This targets both IK tokenization and IK index construction.
- Avoid copying English tokens when normalization returns a slice of the
  caller-owned input document. Tokens in the normalizer's temporary buffer
  still get copied into the parse allocator before the next token. Address and
  length checks establish that a reused slice is entirely inside the document.

The three integrated feature solutions and prior startup fix are retained.
Official benchmark data, SQL, timing, scoring and baseline files are unchanged.

## Validation status

At the user's explicit request, this revision has **not been compiled or
runtime-tested locally**. Only source-path inspection, diff hygiene and Git
tree consistency checks were performed. Earlier v3 test and timing results do
not validate these changes. Compilation, semantic compatibility and performance
must be confirmed by the platform; no score or speedup is claimed for v4.
