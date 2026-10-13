# ACE COMBAT 8 Third-Person HUD

A Windows x64 launcher that keeps the game's native first-person flight
instruments updating in third-person chase view: pitch ladder, compass,
speed/altitude tapes, and aircraft reference mark.

The screenshot below was captured in Free Flight on 2026-10-13 using v0.1.1
on the supported Steam build. It shows the F/A-18C in third-person chase view
with the pitch ladder, compass, speed/altitude tapes, and aircraft reference
mark visible. See [validation](docs/VALIDATION.md) for the recorded checks
and remaining visual tests.

![F/A-18C in third-person chase view with the native flight HUD enabled](docs/images/third-person-hud.jpg)

*Free Flight, San Salvacion North Coast. Original 3840 × 2160 Steam screenshot.*

## Compatibility

- Windows x64 and the Steam edition of ACE COMBAT 8.
- Steam app **2288340**, build **25201480** only.
- Supported `AceCombat8.exe`: **555,102,832 bytes**, SHA-256
  `51510E2A520565DBE81FB0D569E95CD4393077ACAAA859371489B80B8128829F`.
- Intended for offline campaign/single-player use. The launcher refuses
  detected anti-cheat processes, modules, or a protected child process.

The exact executable hash and instruction contexts are checked before any
memory change. A different game update requires a new compatibility review.

## Download and install

Download the v0.1.1 ZIP from [Releases](https://github.com/thunderrun/ace-combat-8-third-person-hud/releases)
and extract it into a separate writable folder. Open `dist/v0.1.1` inside the
extracted folder to find the launcher and short instructions. The same launcher
is available at `dist/v0.1.1/AC8ThirdPersonHUD.exe` in this repository.

1. Close ACE COMBAT 8. Leave Steam running and signed in.
2. Run `AC8ThirdPersonHUD.exe` from the extracted folder.
3. Enter campaign or a single-player free mission and switch to chase view
   using your configured camera-cycle control.
4. Keep the launcher open until the game exits. While the game has focus,
   **Ctrl+Alt+F8** toggles the HUD gates for comparison.

The default game path is
`C:\Program Files (x86)\Steam\steamapps\common\ACE COMBAT 8\Game\Binaries\Win64\AceCombat8.exe`.
For a different Steam library, use PowerShell:

```powershell
.\AC8ThirdPersonHUD.exe --game 'D:\SteamLibrary\steamapps\common\ACE COMBAT 8\Game\Binaries\Win64\AceCombat8.exe'
```

No files need to be copied into the game folder. No UE4SS loader is required.
Logs and status JSON are created in `logs` beside the launcher. Use
`--output PATH` to choose another log directory.

## Disable and remove

Exit the game, then close the launcher. Start the game normally through Steam
to use the original HUD. Delete the extracted mod folder to remove the mod.

The four changes exist only in the launched process's memory and disappear
when that process exits. The launcher does not patch the game executable or
assets on disk and does not write saves; normal gameplay may update saves.

## Check and build

These commands do not launch the game:

```powershell
.\dist\v0.1.1\AC8ThirdPersonHUD.exe --verify
.\dist\v0.1.1\AC8ThirdPersonHUD.exe --self-test
```

`--verify` reads the installed executable and checks its full hash, PE layout,
Steam build, and all four instruction contexts. `--self-test` runs 13 checks
against a synthetic child process, including rollback and page protections.

To compile the original source, install Visual Studio 2022 C++ Build Tools
with the Windows SDK, then run from the repository root:

```powershell
.\scripts\build-launcher.ps1 -Test -Output .\outputs\AC8ThirdPersonHUD-rebuilt.exe
```

The script expects the standard Visual Studio 2022 Build Tools installation
path; edit `$devCommand` for a different installation. Compiler timestamps
can make rebuilt binaries differ in hash.

## Implementation and evidence

[src/launcher.cpp](src/launcher.cpp) contains the launcher, checks, hotkey,
transactional memory updates, and synthetic tests.
[docs/PATCHES.md](docs/PATCHES.md) describes the four changes.
[docs/VALIDATION.md](docs/VALIDATION.md) separates recorded observations,
publication checks, and user-reported success.

The camera getter can also return value 2 when context is missing. The original
player and panel guards remain in place; replay, transitions, and rendering
alignment need their own visual assessment.

This repository contains the original mod source and launcher. Users supply
their own game installation. Game binaries, assets, saves, local analysis
copies, and third-party tools are excluded.

References consulted during the original investigation:

- [AC8 Mod Control](https://github.com/JustBlan1984/AC8-Mod-Control)
- [AC8 UEVR UI fix](https://github.com/lovezzzxxx/ace-combat-8-uevr-ui-fix)
- [Ghidra](https://github.com/NationalSecurityAgency/ghidra)
