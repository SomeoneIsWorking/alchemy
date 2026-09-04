# Project goals

## G001 — One native Alchemy engine boundary used by both titles

Provide reusable Alchemy/Gap engine semantics that the X-Men Legends II and
Marvel: Ultimate Alliance gameplay products both actually consume.

Success conditions:

- X-Men 2 resolves one pinned revision, links the relevant shared targets, and
  executes their APIs through a title-local binding, using an optional
  `alchemy/x86` adapter only where the shared contract needs x86port context.
- The first candidate `alchemy_input` integration matches the retained X-Men 2
  DirectInput path for button bits, pressure, axes, callbacks, lifecycle, and
  stable device identity before that old concrete path is retired.
- After every X-Men 2 project goal is verified, MUA links and exercises the
  proven shared contracts through a title-local binding and an optional
  `alchemy/x360` adapter that consumes the Xbox 360 host's public interfaces.
- Each extracted or extended service is verified against both title evidence
  before being called cross-title; offline tools and checkout presence alone do
  not satisfy consumer adoption.

Constraints: the neutral core has zero x86port/Xbox 360 host dependency.
Platform adapters are separately selectable and own only reusable Alchemy-to-
host translation. Title executables, hashes, guest addresses and object
layouts, native override registration, player policy, and other title-specific
behavior remain in their consuming repositories. MUA work remains deferred
until X-Men 2 is complete.

Non-goals: declaring a complete engine from the current format/tool library;
inventing a generic renderer, scene graph, or resource framework ahead of a
proven consumer contract; rewriting proven C parsers merely to change language.

Contributing state items: S001-S012, S014.

## G002 — A maintainable and enforceable shared runtime surface

Keep the shared boundary cohesive, portable, configurable, and difficult for a
consumer or coding agent to bypass.

Success conditions:

- Pure, proven C parsers remain small production seams; new stateful engine
  owners and adapters use focused C++20 RAII classes, explicit dependencies,
  narrow APIs, and composition.
- Shipping library code receives typed immutable options and emits typed
  diagnostic events through an injected observer. The application maps those
  events once into its configurable Lucent logger. Library code contains no
  direct environment reads, stderr writes, platform debug printing, or
  title-specific diagnostic switches.
- The normal verifier rejects forbidden dependency edges, direct diagnostics,
  out-of-owner configuration reads, title vocabulary in shared runtime code,
  and source-file growth beyond the project limits.
- Dependency resolution has one authority per consumer and pins one immutable
  revision for tools and runtime targets together.

Constraints: configuration ingestion stays at an application/tool boundary;
the shared library receives only the validated values it needs. Interfaces are
generalized only after a concrete title proves the contract.

Non-goals: service locators, global mutable configuration, inheritance-heavy
frameworks, catch-all managers/utilities, or duplicate per-title copies.

Contributing state items: S004, S013, S014.
