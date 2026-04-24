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
`gcc -shared -fPIC -o libcalc.so libcalc.c` manually, and expects the `.so` to
be present in `lib/` at every `nix build` (the suggested `.gitignore` does not
exclude it, and Part 2 Step 5.2 reminds you to rebuild it if missing).

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

### 2. UI's `flake.lock` is not committed

**Tutorial (Part 1 Step 3.1, Part 2 Step 5.1):** `git add flake.lock` after
`nix flake update`.

**Here:** we commit the *core* module's `flake.lock` as the tutorial says
(all its inputs are `github:` refs — lockable), but `logos-tictactoe-ui/flake.lock`
is `.gitignore`d and regenerated on every build.

**Why:** we picked the tutorial's Option B (Part 2 Step 5.2) for the UI→core
flake input — `tictactoe_solo_ai.url = "path:../logos-tictactoe-solo-ai"` —
because both modules are siblings in this repo. With a sibling `path:../`
input, Nix records the input as
`{"path":"../logos-tictactoe-solo-ai","type":"path"}`, which pure-eval mode
rejects as "unlocked" because relative paths have no narHash. CI fails on
the committed lock. The transitive Qt / module-builder pins are still
reproducible via the core module's lock.

## CI

CI uses `lgs basecamp` commands end-to-end (see `.github/workflows/build.yml`).
Each push builds both modules, both variants (`lgx` + `lgx-portable`),
producing four `.lgx` artifacts. Tags produce a GitHub release with all four
files attached.
