# Native HUD changes

Version 0.1.1 targets Steam build 25201480 and the exact executable hash in the
README. Addresses below are RVAs relative to the loaded game module.

| Instrument | Immediate RVA | Original byte | Replacement byte |
| --- | --- | --- | --- |
| Pitch ladder | `0x72F15CD` | `02` | `03` |
| Compass | `0x7243E1B` | `02` | `03` |
| Speed and altitude tapes | `0x72F0E5B` | `01` | `02` |
| Aircraft reference / whisky mark | `0x72F0587` | `01` | `02` |

The comparisons originally admit camera values 0 and 1. The replacement
immediates also admit value 2. All conditional branches, owning-player,
aircraft-type, and panel checks remain intact. Camera selection is unchanged.
The synthetic suite checks all 256 byte values and confirms that only value 2
changes acceptance at each of the four gates.

The launcher checks four 48-byte instruction contexts, temporarily changes
page protections, writes four single-byte immediates, reads back the result,
and restores page protections. It supports toggling and guarded rollback.
These short compatibility signatures are embedded in the source; no game
executable, asset archive, or decompiler dump is distributed.

Native HUD rendering includes a separate 3D/widget-to-texture path. Accepting
chase view in the update routines alone does not prove visibility or alignment.
The camera provider's missing-context fallback can also be 2; transition and
replay behavior therefore remain visual test questions.
