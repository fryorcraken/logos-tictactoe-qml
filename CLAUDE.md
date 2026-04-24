# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A worked example of the [logos-tutorial](https://github.com/logos-co/logos-tutorial)
(Part 1 "wrap a C library in a core module" + Part 2 "QML-only UI module"),
driven end-to-end with [`lgs`](https://github.com/logos-co/logos-scaffold)
instead of the tutorial's raw `nix flake init` / `lgpm install` / `nix build`
commands. Two sibling Logos modules:

- `logos-tictactoe-solo-ai/` — **core** module. Pure C game logic + minimax
  (`lib/libtictactoe.{c,h}`) wrapped in a Qt plugin (`src/tictactoe_plugin.{h,cpp}`,
  `src/tictactoe_interface.h`). Exposes `newGame`, `play`, `getCell`, `status`,
  `currentPlayer`, `vsAiEnabled` as `Q_INVOKABLE` methods.
- `logos-tictactoe-ui/` — **UI** module. Single `Main.qml` that talks to the
  core via `logos.callModule("tictactoe_solo_ai", method, args)`.

Both modules build to `.lgx` artifacts loaded by `logos-basecamp`.

## Common commands

All development goes through `lgs` (pinned in `scaffold.toml` via the
`feature/basecamp` branch — not yet on master). Prerequisites: `nix` with
flakes, `lgs` on `$PATH`.

```bash
# First-time setup
lgs basecamp setup          # Fetch & build basecamp + lgpm, seed alice/bob profiles

# Build + install both modules into every profile
lgs basecamp install

# Launch basecamp for a profile; tictactoe icon in the launcher
lgs basecamp launch alice

# Re-capture modules from sub-flakes into scaffold.toml
lgs basecamp modules
```

Direct `nix` builds (useful when iterating on one module):

```bash
nix build 'path:./logos-tictactoe-solo-ai#lgx'
nix build 'path:./logos-tictactoe-ui#lgx'
```

There are no unit tests and no lint step; verification is "the launcher loads
the module and the board plays correctly."

## Architecture gotchas (non-obvious)

These are the things that will bite you if you treat this like the tutorial
says to. All are documented in more detail in `README.md` (§ Deviations).

1. **`libtictactoe.so` is NOT committed and NOT built by `nix build`'s default
   `vendor_path` handler.** The core flake.nix has a `preConfigure` hook that
   `gcc -shared -fPIC`s the `.so` in-sandbox. Removing the hook → plugin links
   against undefined `tictactoe_*` symbols and crashes at first call.
   Upstream bug: [logos-module-builder#83](https://github.com/logos-co/logos-module-builder/issues/83).

2. **`initLogos` is `Q_INVOKABLE` and NOT marked `override`.** This is the
   tutorial's mandated pattern (`tutorial-wrapping-c-library.md` §2.5 and
   Troubleshooting), not a bug. `PluginInterface` does not declare
   `initLogos` virtual — it is invoked reflectively via `QMetaObject`. Do
   not "fix" this by adding `override` back; the comment at
   `tictactoe_plugin.h:25` exists to stop that refactor.

3. **Inside `initLogos`, assign to the global `logosAPI`, not a member.**
   Also tutorial-mandated (`tutorial-wrapping-c-library.md` §2.6 and
   Troubleshooting "initLogos stores API pointer in wrong variable"). The
   `logos_api.h` macros expect the module-global symbol to be set; a class
   member silently breaks `callModule` dispatch.

4. **UI's `flake.lock` is `.gitignore`d and the UI flake input uses `path:`.**
   Tutorial Part 2 Step 5.2 "Option B" — sibling local-dev recipe. Because
   `path:../` inputs have no narHash, committing the UI lock fails CI in
   pure-eval mode. Core module's lock is committed (github: inputs are
   lockable).

5. **`logos.callModule` return values in QML are `QVariant`** — always coerce
   with `Number(...) || 0` before comparing, and guard against
   `typeof logos === "undefined"` for the QML-preview path.

## Working with `lgs` basecamp state

Local state lives under `.scaffold/` (basecamp profiles, logs, wallet). Two
known `lgs` footguns (details in `README.md` § Known issues):

- `lgs basecamp reset` does **not** detect a basecamp launched in another
  shell and will happily wipe its profiles out from under the running
  process → segfault. If a launcher is running anywhere, kill it before
  `reset`.
- `lgs basecamp reset` also clears `[[basecamp.modules]]` from
  `scaffold.toml`, so `build-portable` will refuse until you re-run
  `lgs basecamp modules`.

## CI

`.github/workflows/build.yml` installs `lgs` from the `feature/basecamp`
branch, runs `lgs basecamp modules` + `lgs basecamp build-portable`, and
additionally builds the dev `.lgx` variants via direct `nix build`. Four
artifacts per push (both modules × {portable, dev}); tags cut a GitHub
release with all four attached. If `build-portable` output format changes,
update the `grep -oE '/nix/store/[^ ]*\.lgx'` extraction in the "Stage
artifacts" step.
