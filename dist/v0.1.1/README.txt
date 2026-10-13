ACE COMBAT 8 Third-Person HUD v0.1.1

Windows x64. Supported Steam app 2288340, build 25201480 only.
Expected game EXE SHA-256:
51510E2A520565DBE81FB0D569E95CD4393077ACAAA859371489B80B8128829F

1. Extract into a separate writable folder. Close ACE COMBAT 8.
2. Leave Steam running and signed in, then run AC8ThirdPersonHUD.exe.
3. Enter offline campaign/single-player and switch to chase view using
   your configured camera control.
4. Keep the launcher open. Ctrl+Alt+F8 toggles the HUD gates while the
   game has focus.

For a different Steam library:
  .\AC8ThirdPersonHUD.exe --game 'D:\SteamLibrary\steamapps\common\ACE COMBAT 8\Game\Binaries\Win64\AceCombat8.exe'

Read-only installed-build check:
  .\AC8ThirdPersonHUD.exe --verify
Synthetic checks without launching the game:
  .\AC8ThirdPersonHUD.exe --self-test

The launcher changes four bytes only in its launched game process's memory.
No game-folder installation or UE4SS is required. Logs are written in logs\.
For removal, exit the game and launcher, then delete this extracted folder.
Normal Steam launch uses the original HUD. The launcher does not write saves.

The owner reports successful in-game use. v0.1.1 has static and synthetic
verification; publication did not independently observe chase-view gameplay.
See README.md and docs\VALIDATION.md in the release ZIP for testing scope,
source, and build instructions.

https://github.com/thunderrun/ace-combat-8-third-person-hud
