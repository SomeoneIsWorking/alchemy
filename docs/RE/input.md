# The Alchemy input subsystem

`igControllerManager` / `igController` and their platform subclasses, recovered
from the ARK class registry of `libIGDisplay.dll` (X-Men Legends II PC, Alchemy
3.2).

**This is engine, not game.** It is written up here rather than in a port
because the abstract half is the same engine on every platform Alchemy targets,
so an Xbox, PS2 or Xbox-360 title built on it — Marvel Ultimate Alliance
included — meets the same two interfaces with a different concrete class behind
them. Only the concrete class is per-platform work.

## The class graph

Every Alchemy class describes itself in one 11-argument `igArkRegister` call
(see [`ark.md`](ark.md)), so this is read out of the binary rather than guessed.
`libIGDisplay.dll` registers 19 classes at 19 call sites in 549 functions, with
**0 sites whose arguments could not be fully recovered** and **0 disagreements**
between the two independent readings of abstractness (the `isAbstract` argument,
and whether `retrieveVTablePointer` is NULL).

| class | abstract | instance size | registrar | ARK meta slot |
|---|---|---|---|---|
| `igControllerManager` | yes | 12 | `0x10003a20` | `0x10021c20` |
| `igController` | yes | 16 | `0x10003bc0` | `0x10021c30` |
| `igWin32ControllerManager` | no | 12 | `0x10004440` | `0x10021cf4` |
| `igWin32Controller` | no | **148** | `0x10004570` | `0x10021cfc` |
| `igControllerList` | no | 20 | `0x100037b0` | `0x10021bd4` |
| `igControllerStack` | no | 20 | `0x100035d0` | `0x10021bcc` |

Addresses are at the module's PREFERRED base `0x10000000`. Every `libIG*.dll`
links there, so an address alone does not identify a module — always qualify it.

## The substitution point, which is the whole reason to read this

`_Meta+0x3c` carries each abstract class's concrete delegate. In this build:

```
igController         -> igWin32Controller
igControllerManager  -> igWin32ControllerManager
igInterfaceManager   -> igDefaultInterfaceManager
igWindow             -> igWin32Window
```

Four such bindings, all four recovered. A port therefore does **not** replace
`igController`: it supplies the concrete class the abstract one delegates to,
and everything above the interface is untouched. That is the seam a new
platform is added at, and it is why input work here is groundwork for another
title rather than for this one only.

`igWin32Controller` being 148 bytes against the abstract `igController`'s 16
says the platform subclass owns essentially all of the state — the device
handles and the physical-value array — while the interface owns almost none.
Expect the same shape on any other platform's subclass.

## What is NOT established here

- **No method semantics.** This is the class graph and the substitution map. The
  vtable layouts, which slot reads a physical value, and what the per-player
  controller array looks like are separate work. On the Xbox build the
  equivalent chain IS resolved end to end (manager `sub_00160E60` → slot `+0x4c`
  `sub_0015F940(player)` → controller slot `+0x10` `sub_0015F5B0`, reading
  `[this + index*4 + 0x2fc]` of a 30-float array) — that is recorded as claim
  C192 in the X-Men Legends II port, and the PC equivalents are not yet mapped
  onto it.
- **No MUA verification.** MUA is an Xbox 360 title: PowerPC and big-endian, so
  the machine code does not transfer and neither do these addresses. What
  transfers is the architecture — the two interfaces, the `_Meta+0x3c`
  substitution, and the expectation that the concrete subclass holds the state.
  Nothing here has been checked against an MUA binary.
- **Endianness.** The IGB readers in this repo do no byte swapping, which is
  correct for PC/Xbox/PS2 and wrong for a 360 title. That gap is real and is not
  closed by this document.

## Reproducing it

```sh
python3 tools/ark_classes.py <module>.json <module>.dll <module>.iat --json out.json
```

The tool reports its denominators — call sites found, classes recovered, sites
whose arguments could not be recovered, and whether the two abstractness
readings agree — so a run that recovered nothing cannot be mistaken for a module
that registers nothing.
