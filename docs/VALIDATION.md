# Validation and testing scope

## Version 0.2.0, 13 October 2026

The final v0.2.0 launcher passed the supported executable's full SHA-256, size, Steam build, AMD64 PE layout, startup comparison/branch target, and all five instruction-context checks. Its source and build script match the tested package, subject to the repository's text newline normalization.

All **21 synthetic cross-process checks passed**. Coverage includes the four camera gate semantics, full-file hashing and unsupported inputs, guarded writes and exact readback, rollback and page protections, mixed intro/HUD transactions, rewritten-comparison rejection, foreign-byte preservation, and HUD toggles that leave startup skip active. A synthetic child also executes the original `JZ` and patched `JNO`: the patched branch reaches completion for zero, nonzero, and high-bit counts, and rollback restores the original zero-only behavior.

A live session with v0.2.0 reached the title screen and main menu. The user directly confirmed that the startup logos were skipped. Launcher readback showed startup skip and all four HUD patches active through the session. The game exited normally with code **0**, and the installed executable's hash was unchanged before and after.

These observations establish the tested startup result. Continuous footage of the earliest startup frames was not recorded. v0.2.0 flight HUD visibility, changing telemetry, alignment, a visual enabled/disabled comparison, camera cycling, and campaign transitions were not assessed in that session. Active memory bytes alone do not establish those visual results.

The earlier standalone experimental data-asset intro package did not skip logos in the reported session. Its three owned files were removed after that session exited; v0.2.0 uses a process-memory branch patch and needs no asset override or loader.

The release candidate launcher SHA-256 is:

```text
5880CC277A9A3D08F45A54E0170BEFD0FE21886A120DE74ECBA79880B1FAB3F1
```

Sanitized checks are recorded in [PUBLICATION-CHECKS.json](PUBLICATION-CHECKS.json). Session PIDs, personal paths, saves, local logs, and private analysis files are excluded from publication.

## Historical v0.1.1 HUD evidence

The existing [gameplay screenshot](images/third-person-hud.jpg), captured on 13 October 2026 using v0.1.1, shows an F/A-18C from behind in Free Flight at San Salvacion North Coast. The pitch ladder, compass, speed/altitude tapes, and central aircraft reference mark are visible. The displayed heading is 270, speed 620, and altitude 6999.

The original image is 3840 x 2160 pixels, 662,621 bytes, SHA-256:

```text
A38668CBC287B7109CD5A20083A1E687FEE5CD31686CE68C76C5EA8D5D6EB061
```

That frame establishes visibility for v0.1.1, whose four HUD byte changes are identical to v0.2.0. It does not independently verify v0.2.0 flight behavior or establish dynamic telemetry, pitch/bank/FOV alignment, same-flight visual comparison, or transitions. v0.1.1 also passed its original 13 synthetic checks and supported-build verification.

## Remaining visual assessment

Using v0.2.0, check chase-view instruments through changing pitch, bank, heading, speed, and altitude. Compare Ctrl+Alt+F8 enabled/disabled views in the same flight, then check camera cycling, freelook, target tracking, FOV changes, pause/cutscenes, restart, and mission transitions. Check startup skip alone with `--no-hud` as a separate launch mode.
