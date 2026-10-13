# Native startup and HUD changes

Version 0.2.0 targets Steam build 25201480 and the exact executable hash in the README. Addresses are RVAs relative to the loaded game module.

| Change | Byte RVA | Original | Replacement |
| --- | --- | --- | --- |
| Startup notice completion branch | `0x07B4F018` | `84` | `81` |
| Pitch ladder | `0x072F15CD` | `02` | `03` |
| Compass | `0x07243E1B` | `02` | `03` |
| Speed and altitude tapes | `0x072F0E5B` | `01` | `02` |
| Aircraft reference / whisky mark | `0x072F0587` | `01` | `02` |

## Startup skip

`LiveMenuMainNoticeScreenWidget` compares its local sequence count with zero at RVA `0x07B4F010`, immediately followed by `JZ` at `0x07B4F017`. The patch changes the second opcode byte from `84` to `81`, making it `JNO` with the same displacement and completion target, RVA `0x07B4F0D7`.

`CMP x,0` computes `x-0=x`, so its signed overflow flag is always clear. The patched `JNO` therefore takes the existing native completion path for every sequence count. That path requests state 3 and returns through the common cleanup epilogue. The sequence array and its owned references remain intact for normal cleanup.

A 48-byte guard begins at RVA `0x07B4EFF9`; the changed byte is at guard offset 31. The hash and guard also establish the required zero comparison and unchanged target. No campaign cinematic routine is patched. The suite executes a synthetic zero-comparison branch before and after the opcode change, including zero, nonzero, and high-bit counts.

## HUD gates

The original comparisons admit camera values 0 and 1; the replacement immediates also admit value 2. The four HUD bytes are unchanged from v0.1.1. Existing owning-player, aircraft-type, panel, and conditional-branch checks stay intact. The synthetic suite checks all 256 camera byte values and confirms that only value 2 changes acceptance at each gate.

Native HUD rendering includes a separate 3D/widget-to-texture path. Gate acceptance does not by itself prove visibility, alignment, or changing telemetry. The camera provider can return value 2 when context is missing, so replay and transition behavior remain visual test questions.

## Application and recovery

The launcher starts its own game child suspended, validates the remote guards, applies one startup byte and optionally four HUD bytes, reads back every change, restores page protections, and then resumes the child. `--no-hud` selects only the startup byte and does not register the HUD hotkey.

Ctrl+Alt+F8 toggles only the HUD group. Startup readback is checked independently and remains enabled during HUD comparisons. Periodic checks validate selected patches and reject detected anti-cheat modules. Failure recovery restores owned replacement bytes when their surrounding guards still match, preserving unknown bytes. A launch mutex prevents a second copy from starting another game session through the same launcher.

No game files are patched on disk. The source includes short compatibility signatures and synthetic tests; game executable copies, assets, and decompiler dumps are excluded.
