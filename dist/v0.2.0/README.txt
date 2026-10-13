ACE COMBAT 8 Startup Skip + Third-Person HUD v0.2.0

Windows x64. Steam app 2288340, build 25201480 only.
Supported game executable SHA-256:
51510E2A520565DBE81FB0D569E95CD4393077ACAAA859371489B80B8128829F

1. Extract the entire release into a separate writable folder. Close AC8.
2. Leave Steam running and signed in.
3. Run AC8NoIntroHUD.exe or Launch-Intro-and-HUD.cmd for both features.
4. Use offline campaign/single-player. Keep the launcher open until exit.
5. Ctrl+Alt+F8 toggles only the HUD while the game has focus.

For startup skip alone, run Launch-Intro-Only.cmd or:
  AC8NoIntroHUD.exe --no-hud
For a different Steam library:
  AC8NoIntroHUD.exe --game "D:\SteamLibrary\steamapps\common\ACE COMBAT 8\Game\Binaries\Win64\AceCombat8.exe"

No PAK, DLL, or UE4SS installation is needed. The startup sequence is
skipped through an existing completion path; campaign cutscene routines
are unchanged. The four HUD bytes match HUD v0.1.1. No game files, saves,
or input bindings are written by the launcher. Normal Steam launch uses
original startup/HUD behavior. Delete the extracted folder after exiting
the game and launcher to remove it. Runtime logs appear in logs/.

Read-only installed-build check:
  AC8NoIntroHUD.exe --verify
Synthetic checks without launching the game:
  AC8NoIntroHUD.exe --self-test

Supported-build verification and 21 synthetic checks passed. In v0.2.0,
the title/menu were reached, the user confirmed skipped logos, both patch
groups remained active, and exit code was 0. Full v0.2.0 flight HUD visual
assessment and mission-transition checks remain pending. See the release
README.md and docs/VALIDATION.md for the scope and historical HUD image.

After an old experimental intro-data session exits, use that package's
original uninstall tool before switching to this installation-free version.

https://github.com/thunderrun/ace-combat-8-third-person-hud
