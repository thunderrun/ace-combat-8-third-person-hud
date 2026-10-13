# v0.2.0 - Startup Skip + Third-Person HUD

Skip the full startup notice sequence and enable the existing third-person native flight HUD in one launcher. The new `AC8NoIntroHUD.exe` changes one guarded startup opcode and four optional HUD bytes in process memory; no PAK, DLL, or UE4SS installation is required. `--no-hud` runs startup skip alone, and Ctrl+Alt+F8 continues to toggle only the HUD.

Supports Windows x64 Steam app 2288340, build 25201480 with the pinned executable hash. Close AC8, leave Steam running, extract the ZIP, and run `dist/v0.2.0/AC8NoIntroHUD.exe`. Keep the launcher open until the game exits. Use offline campaign/single-player.

Validation: supported-build verification and 21 synthetic checks passed. A v0.2.0 live session reached the title screen/main menu, the user confirmed skipped startup logos, both patch groups stayed active, and the game exited with code 0. Flight HUD motion/alignment and campaign transitions remain unverified in v0.2.0; the repository retains the earlier v0.1.1 chase-view screenshot separately.

The launcher does not edit installed game files or write saves. Existing users of the standalone experimental intro data package should let its old session exit and use its original uninstall tool before switching.
