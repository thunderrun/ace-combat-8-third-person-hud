# Changelog

## 0.2.0 - 2026-10-13

- Add startup notice skip through a guarded one-byte native completion branch patch, with no game-folder installation.
- Combine startup skip and the existing four third-person HUD changes in `AC8NoIntroHUD.exe`; add `--no-hud` for startup skip alone.
- Keep Ctrl+Alt+F8 limited to HUD toggling and add independent startup readback and a launch mutex.
- Expand synthetic validation to 21 checks, including executed branch behavior and feature isolation.
- Record user-confirmed skipped logos, observed title/menu startup, active patch readback, and normal exit 0. Full v0.2.0 flight HUD visual assessment remains pending.

## 0.1.1 - 2026-10-13

- Publish the four-byte third-person HUD launcher with guarded recovery and 13 synthetic checks.
- Document one observed chase-view screenshot showing the native flight instruments.
