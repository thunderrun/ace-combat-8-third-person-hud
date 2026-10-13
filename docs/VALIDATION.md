# Validation and testing scope

## Owner report

On 13 October 2026, the project owner reported completing the first-person HUD
in third-person view mod and requested public publication. This is a
user-reported in-game success. The publication session did not independently
observe chase-view gameplay or establish which launcher version was used for
that successful session.

## Recorded project evidence

The original project records from 13 October 2026 show:

- v0.1.1 was built with Visual C++ 2022. Its source and build script match the
  copies shipped in the final local package by SHA-256.
- v0.1.1 passed full installed-executable hash, size, Steam build, AMD64 PE,
  and four instruction-context verification.
- v0.1.1 passed 13 synthetic cross-process checks, covering camera-value
  semantics, full-file hashing, unsupported inputs, guarded transactions,
  readback, rollback, page-protection restoration, A/B toggling, and ownership
  handling when other code or bytes change.
- An earlier **v0.1.0** live session reached single-player flight training in
  cockpit view. Native instruments were visible. Ctrl+Alt+F8 disabled and
  enabled the patches with exact four-context memory readback at
  06:21:46 UTC and 06:25:08 UTC respectively.
- The four HUD gate changes are identical between those two versions.
  v0.1.1 adds reviewed recovery handling and module-snapshot retries.
- The original records did not verify chase-view visibility, alignment,
  changing telemetry, camera cycling, or gameplay with the final v0.1.1.
  Memory readback is not proof of those visual results.

## Publication checks

The publication checks and binary checksum are recorded in
`PUBLICATION-CHECKS.json`. They are static/synthetic checks and do not establish
an independently observed chase-view gameplay result.

The uploaded launcher is the final local v0.1.1 binary, SHA-256:

```text
EAD79FB263A6D397476F9DB9207D15DC7E1687B3DBE49AAA8A80FAD9FDDA4725
```

Local saves, save hashes, machine-specific logs, firewall records, screenshots,
analysis copies, and third-party toolchains are omitted from the public package.
The publication process does not install or replace the working mod, change
game files, launch the game, or alter anti-cheat/security settings.

## Visual assessment

For independent visual validation, check chase-view pitch ladder, compass,
speed and altitude tapes, and reference mark during changing pitch, bank,
heading, speed, and altitude. Compare enabled/disabled views in the same flight,
then check camera cycling, freelook, tracking, FOV, pause/cutscenes, restart,
and mission transitions using the final v0.1.1 launcher.
