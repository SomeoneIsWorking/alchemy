# Alchemy engine guidance

The repository-wide rules in `../../AGENTS.md` apply here. Consult
`docs/codemap.md` before changing a subsystem and update it in the same commit.

## Product boundary

This repository owns reusable Alchemy/Gap engine semantics: IGB containers and
their typed payloads, XMLB, FB packages, animation codecs, audio container
formats, the abstract input model, and platform-neutral asset inspection.

It does not own a game's executable, guest addresses, a console kernel, a
static recompiler, title-specific scripts, or a shipping renderer. Those live
in the consuming game port or a shared console-host repository. A behavior is
shared only after its engine ownership is evidenced; do not move title policy
here because two games happen to call it.

## Compatibility evidence

- Treat CPU byte order, container byte order, and encoded GPU/audio payload
  order as separate facts. Never infer one from another.
- A parser opening a container proves structure only. Texture decode,
  animation semantics, mesh topology, and rendering each need their own
  positive and negative checks.
- Real-disc corpus checks accept explicit paths or environment variables and
  keep all extracted data under ignored `scratch/`. Tests never contain or
  download copyrighted assets.
- Every corpus report prints how many archives, embedded entries, objects, and
  payloads it examined, plus every unsupported case. Zero matches is a refusal,
  not success.
- `x2_*`, `X2_*`, and `X2VIEW_*` names are extraction residue, not an ownership
  boundary. New public APIs use `alchemy_` or the engine's factual `ig*`
  vocabulary; migrate consumers atomically when retiring an old name.

## Structure

Keep container parsing, typed mesh/image interpretation, animation, input,
archive tooling, and viewers as separate owners. Tests call production seams;
they do not reimplement byte-order or layout rules. New first-party source
files are capped at 500 lines. Existing larger files are frozen until a
cohesive owner is extracted, then their ceiling is lowered.

Build first-party C with Clang. `scratch/build-clang` is the normal local build
tree; the checked-in `build/` directory predates this rule and its GNU results
are not valid verification.
