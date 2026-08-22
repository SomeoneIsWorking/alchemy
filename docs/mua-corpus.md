# Marvel Ultimate Alliance corpus gate

`tools/mua_corpus.py` is the reproducible compatibility gate for the supplied
PS2 release and Xbox 360 Gold Edition. It starts from both disc images so an
old partial extraction cannot produce a green result. No disc path or derived
game asset is tracked; staging happens under ignored `scratch/mua-corpus/`.

```sh
python3 tools/mua_corpus.py \
  --ps2-iso /path/to/mua-ps2.iso \
  --x360-iso /path/to/mua-gold-x360.iso
```

The gate requires `7z` for the PS2 ISO and `xdvdfs` for the Xbox 360 XISO. It
refuses if either input, either tool, the shipping `igb_dump`, or any of these
four archive layers is missing:

- PS2 `Z/ASSETSFB.WAD`
- Xbox 360 `z/assetsfb.zip`
- Gold Edition `muadlc_content.zip`
- Gold Edition `muadlc_titleupdate.zip`

The PS2 WAD contains a complete ZIP end record followed by a second truncated
record signature. `tools/alchemy_archives.py` selects the complete directory
with the greatest valid entry count. `tools/extract_wad.py` uses that same
reader; the corpus gate does not carry a second WAD implementation.

FB packages are expanded through the vendored raven-formats parser. Every
non-empty `.xmlb` entry is parsed and byte-identically serialized by the
shipping `tools/xmlb.py`; every non-empty `.igb` is opened by the built
shipping `igb_dump`. Zero-byte FB members are reported separately as declared
placeholders, not passed to a parser and not silently discarded. Other formats
are counted by extension as unsupported, so the report distinguishes format
coverage from archive coverage.

## Synthetic discriminator

```sh
python3 tools/mua_corpus.py --selftest
```

The selftest creates all fixtures below ignored `scratch/`. It proves the
positive WAD -> FB -> XMLB path, including repeated FB member names, and proves
four negative answers: malformed-archive refusal, missing-corpus refusal,
truncated-FB refusal, and invalid-XMLB failure. It reports `N of N`; a missing
test cannot read as success.

## Measured supplied corpus (2026-08-22)

The explicit PS2 and Xbox 360 Gold Edition images supplied in the ROM corpus
produced:

| Layer | Result |
|---|---:|
| Required archives | 4 / 4 opened |
| Archive file entries | 2,949 |
| FB packages | 1,087 / 1,087 parsed |
| FB embedded entries | 40,158 |
| Duplicate-path FB occurrences retained | 129 |
| Empty FB packages | 8 |
| XMLB payloads | 14,348 / 14,348 parsed and byte-identically round-tripped |
| XMLB nodes | 283,057 |
| IGB containers | 14,187 / 14,187 opened |
| IGB objects | 7,152,367 |
| IGB image payloads discovered | 82,377 |
| Declared zero-byte payload placeholders | 347 |
| Parser failures | 0 |

The individual archive denominators were 1,104 PS2 WAD entries, 1,133 base
Xbox 360 entries, 651 Gold DLC content entries, and 61 Gold title-update
entries. There were 13,138 entries in formats this gate does not semantically
parse; the command reports their full extension histogram.

The FB denominator counts every occurrence, including repeated paths; the
shipping raven-formats parser exposes them through `iter_entries` rather than
losing earlier occurrences when its extraction view overwrites a name. This
proves archive traversal, FB extraction, XMLB byte identity, and IGB
container/schema opening. It does **not** prove mesh meaning, animation output,
or decoded texture pixels. In particular, the 82,377 images are a discovery
denominator, not evidence that every platform raster format is decoded.

The measurement is falsified by a different disc image, a change to
`tools/alchemy_archives.py`, the vendored FB parser, `tools/xmlb.py`, or the IGB
parser/build used by `igb_dump`; rerun the full explicit-ISO command after any
of those changes.
