# igblib (vendored)

KaikoClanworth1's pure-Python IGB container reader ("Based on the mateon1
gist parser and Alchemy 5.0 SDK headers", per its own docstrings), vendored
here so every consumer of this repo — the XML2 port's font pipeline, this
repo's `tools/extract_font_igb.py` — resolves it from one place instead of a
gitignored `scratch/ref` copy per machine.

No LICENSE file accompanied the copy this was taken from; upstream is
credited in this README instead. If you have the original repository, prefer
it and diff against this snapshot.

Consumers put `vendor/` on `sys.path` (see the xmen2 port's
`tools/make_pad_font.py` and this repo's `tools/extract_font_igb.py`) and
`import igblib`.
