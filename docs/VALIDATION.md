# Validation and testing scope

## Owner report

On 13 October 2026, the project owner reported completing the first-person HUD
in third-person view mod and requested public publication. This is a
user-reported in-game success. The publication session did not independently
observe chase-view gameplay during the initial publication or establish which
launcher version was used for that reported successful session. A later
independent screenshot with v0.1.1 is recorded below.

## Observed v0.1.1 chase-view screenshot

On 13 October 2026 at **08:44:05 UTC**, a Steam screenshot captured the installed
game running through **v0.1.1** in Free Flight, **01 Jokers Wild / San Salvacion
North Coast**, using the **F/A-18C**. The aircraft is visibly shown from behind
in third-person chase view. The frame shows the pitch ladder, compass with
heading **270**, speed tape displaying **620**, altitude tape displaying
**6999**, and central aircraft reference mark.

The launcher reported `running_patched`, `memory_patch_active: true`, and all
four patched contexts verified at **08:43:46 UTC**. The game was paused after
the screenshot. The selected [gameplay image](images/third-person-hud.jpg) is
3840 x 2160 pixels, 662,621 bytes, with SHA-256:

```text
A38668CBC287B7109CD5A20083A1E687FEE5CD31686CE68C76C5EA8D5D6EB061
```

This establishes visibility of those instruments in one chase-view frame using
the final v0.1.1 launcher. It does not establish changing telemetry, alignment
across pitch/bank/FOV, a same-flight visual enabled/disabled comparison, or
transition behavior.

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
motion or alignment results. The later still-image observation is recorded
separately in that file.

The uploaded launcher is the final local v0.1.1 binary, SHA-256:

```text
EAD79FB263A6D397476F9DB9207D15DC7E1687B3DBE49AAA8A80FAD9FDDA4725
```

The selected Steam gameplay image is included. Local saves, save hashes,
machine-specific logs, firewall records, analysis copies, and third-party
toolchains are omitted. The screenshot session launched the installed game
through v0.1.1; the launcher changes four process-memory bytes and does not
patch game files or write saves. Normal gameplay may update saves. This
documentation update does not replace the distributed launcher or change
anti-cheat/security settings.

## Visual assessment

For further visual validation, check the chase-view instruments during changing
pitch, bank, heading, speed, and altitude. Compare enabled/disabled views in the
same flight, then check camera cycling, freelook, tracking, FOV, pause/cutscenes,
restart, and mission transitions using the final v0.1.1 launcher.
