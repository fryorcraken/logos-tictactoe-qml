# logos-tictactoe-qml

A single-player tic-tac-toe Logos module with a minimax AI opponent. Built as a
worked example of the [logos-tutorial](https://github.com/logos-co/logos-tutorial)
Part 1 (wrapping a C library in a core module) combined with Part 2 (QML-only
UI module) — but scaffolded and driven end-to-end with
[`lgs`](https://github.com/logos-co/logos-scaffold) instead of the tutorial's
`nix flake init` / `lgpm install` / `nix build` commands.

## Layout

```
logos-tictactoe-qml/
├── logos-tictactoe-solo-ai/   # Core module: C game logic + minimax AI + Qt plugin
│   ├── lib/libtictactoe.{c,h} # Pure C game logic, minimax AI
│   ├── src/                   # Qt plugin wrapper (tictactoe_interface.h, tictactoe_plugin.{h,cpp})
│   ├── CMakeLists.txt
│   ├── flake.nix
│   └── metadata.json
└── logos-tictactoe-ui/        # QML UI module — calls core via logos.callModule
    ├── Main.qml               # Board + single "New Game" button (human X, AI O)
    ├── icons/tictactoe.png    # App icon (X, O, robot face)
    ├── flake.nix
    └── metadata.json
```

## Quick start

Prerequisites: nix, [`lgs`](https://github.com/logos-co/logos-scaffold) on `$PATH`.

```bash
# From the repo root:
lgs basecamp setup          # Fetch & build basecamp + lgpm, seed alice/bob profiles
lgs basecamp install        # Build both modules' .lgx, install into every profile
lgs basecamp launch alice   # Launch basecamp for alice; click the tictactoe icon
```

## Tutorial steps replaced by `lgs`

This repo follows the tutorial Part 1 + Part 2 step-for-step, with the
following substitutions:

| Tutorial step / command | `lgs` equivalent used here |
|---|---|
| `nix flake init -t ...#with-external-lib` (scaffold core module) | `lgs new` (equivalent scaffold invocation) |
| `nix flake init -t ...#ui-qml` (scaffold UI module) | `lgs new` |
| Manual `lgpm install --file ...` | `lgs basecamp install` (auto-discovers sub-flakes) |
| Manual `nix build .#lgx` per module | `lgs basecamp install` (builds both variants for every captured source) |
| Manual basecamp launch from AppImage | `lgs basecamp launch alice` |

## Deviations from the tutorial

Kept minimal — only where we hit a concrete problem following the tutorial as
written. Each deviation lists: what the tutorial does, what we do instead, and
why.

### 1. Vendored `.so` built in-sandbox, not committed

**Tutorial (Part 1, Step 1.5):** instructs the developer to run
`gcc -shared -fPIC -o libcalc.so libcalc.c` manually before `nix build`, and the
project tree shows `lib/libcalc.so` as a tracked file next to `libcalc.c`.

**Here:** we compile `libtictactoe.so` in-sandbox via a `preConfigure` hook in
`logos-tictactoe-solo-ai/flake.nix`, keep only `.c` / `.h` in git, and rely on
module-builder's `vendor_path` handler to stage the freshly-built `.so` into
the build.

**Why:** reproducibility (all builds use the nix-pinned `gcc`) and no binary
artefacts in git. Also a current necessity — module-builder's `vendor_path`
handler ignores the metadata `build_command` field
([logos-co/logos-module-builder#83](https://github.com/logos-co/logos-module-builder/issues/83)),
so without the hook `.#lgx` links against undefined `tictactoe_*` symbols and
crashes at first call.

### 2. `initLogos` not marked `override` in the core plugin

**Tutorial + `with-external-lib` scaffold template:** declare
`void initLogos(LogosAPI* api) override;` on the plugin class.

**Here:** we drop `override` and mark the method `Q_INVOKABLE`.

**Why:** the `PluginInterface` base class shipped with the unpinned
`logos-module-builder` default branch does NOT declare `initLogos` as a virtual
method. The scaffold template therefore does not compile as-shipped —
**scaffold template bug, to be reported upstream.**

### 3. UI flake references core via `path:` instead of `github:`

**Tutorial:**
```nix
calc_module.url = "github:logos-co/logos-tutorial?dir=logos-calc-module";
```

**Here:**
```nix
tictactoe_solo_ai.url = "path:../logos-tictactoe-solo-ai";
```

**Why:** the tutorial's core module lives in a separate public repo that's
already been pushed. Ours is a sibling directory in the same repo being built
alongside the UI, so `path:` is the correct reference — a `github:` URL would
pull master (or a stale pin) instead of picking up local edits to the sibling
core module.

**Follow-on deviation:** the tutorial instructs `git add flake.lock` after
`nix flake update`, and we do commit the *core* module's `flake.lock` (all
its inputs are `github:` refs — lockable). We do **not** commit the UI's
`flake.lock`: with a sibling `path:../` input, Nix records the input as
`{"path":"../logos-tictactoe-solo-ai","type":"path"}`, which pure-eval mode
rejects as "unlocked" because relative paths have no narHash. CI fails on
this. `logos-tictactoe-ui/flake.lock` is `.gitignore`d; the transitive Qt /
module-builder pins are still reproducible via the core module's lock.

## Known issues hit while dogfooding `lgs`

- **`lgs basecamp reset` does not detect a launched basecamp that was started
  in a separate shell.** It reports `kill: none (no live basecamp tracked)`
  and wipes `.scaffold/basecamp/profiles/` out from under the running window,
  segfaulting the `logos_host` subprocesses. Repro: `lgs basecamp install` →
  `lgs basecamp launch alice` (in terminal A) → `lgs basecamp reset` (in
  terminal B). Fix candidate: fall back to a `pgrep -f` against the profile
  paths before wiping, or record the launcher PID on `launch` into a lockfile
  that `reset` consults.

- **`lgs basecamp reset` also clears captured modules from `scaffold.toml`,
  which is more than expected.** Intuitively `reset` should only drop
  *installed* state (profiles, live processes) so that `basecamp install` /
  `basecamp build-portable` still work out of the box on the same project.
  Today, after `reset`, `build-portable` refuses with `no project modules
  captured in scaffold.toml; run `basecamp modules` first ... `build-portable`
  operates on captured project sources only — it never discovers.`, forcing a
  manual `lgs basecamp modules` step before any subsequent build. Fix
  candidate: leave `[[basecamp.modules]]` untouched on `reset`, or gate the
  capture-clearing behind an explicit `--clear-captures` flag.

## CI

CI uses `lgs basecamp` commands end-to-end (see `.github/workflows/build.yml`).
Each push builds both modules, both variants (`lgx` + `lgx-portable`),
producing four `.lgx` artifacts. Tags produce a GitHub release with all four
files attached.
