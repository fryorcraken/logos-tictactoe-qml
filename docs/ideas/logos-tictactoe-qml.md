# logos-tictactoe-qml

Repo #1 of a three-repo split mapping our current `logos-module-tictactoe`
work onto the `logos-co/logos-tutorial` structure. Two-module monorepo
mirroring tutorial Part 2 (`logos-calc-module` + `logos-calc-ui`), no
multiplayer, no `delivery_module`, AI opponent in the core module.

## Problem Statement

How might we ship the Part-2-analogue of the tutorial as a standalone
repo — two modules (C core + QML UI) in one monorepo — such that every
build/install/run step goes through `lgs basecamp` and `lgs init` from
day zero, with no manual `lgpm` or `nix build` invocations in the
documented workflow?

## Recommended Direction

Two-subdir monorepo mirroring `logos-co/logos-tutorial`:

```
logos-tictactoe-qml/
├── logos-tictactoe-solo-ai/   # core module (C lib + C++ plugin)
│   ├── flake.nix              # scaffolded via `lgs init`
│   ├── metadata.json          # name: "tictactoe_solo_ai"
│   ├── src/*.cpp,*.h          # Qt plugin, C bindings, minimax AI
│   └── lib/*.c,*.h            # libtictactoe C library
├── logos-tictactoe-ui/        # QML UI module
│   ├── flake.nix              # scaffolded via `lgs init`
│   ├── metadata.json          # name: "tictactoe_ui", dep: ["tictactoe_solo_ai"]
│   ├── Main.qml               # calls logos.callModule("tictactoe_solo_ai", ...)
│   └── icons/tictactoe.png
├── scaffold.toml              # from `lgs init` at repo root
└── README.md                  # `lgs basecamp setup → install → launch`
```

**Core module** (`tictactoe_solo_ai`): game state, win detection, draw
detection, minimax AI. Exposes `newGame(vsAi: bool)`, `play(row, col)`,
`getCell(row, col)`, `status()`, `currentPlayer()`, `vsAiEnabled()`.
When `vsAiEnabled` and it's O's turn, `play()` applies the AI reply
before returning, so the UI only calls `play()` for the human and reads
back the resulting board. Pure sync RPC via `logos.callModule` —
no delivery, no events, no `.rep`, no QRO.

**UI module** (`tictactoe_ui`): single `Main.qml`, hotseat 2-player by
default, "New Game vs AI" button next to "New Game". Status line and
win-line highlight copied from the current `tictactoe-ui-qml/Main.qml`,
minus the multiplayer block and poll timer.

**Scaffold-first constraint**: every build/install/run path goes
through `lgs`. No manual `lgpm install --file` or `nix build .#lgx`
instructions in the README. CI may call `lgs` or fall back to `nix
build` as needed, but end-user documentation is `lgs`-only.

Source material: copy + strip from the current `logos-module-tictactoe`
monorepo (`tictactoe/` → core with multiplayer excised + minimax added;
`tictactoe-ui-qml/Main.qml` → UI with the 95-line multiplayer block
removed). Don't migrate git history — the excisions are aggressive
enough that a fresh `lgs init` lineage is cleaner than a filtered
history rewrite.

## Key Assumptions to Validate

- [ ] `lgs init` (or `lgs basecamp init`) scaffolds both a core-module
  template and a QML-module template. If only one exists, file a
  scaffold issue and pick whichever is available, patching the other
  manually as a one-off.
- [ ] `lgs basecamp install` auto-discovers both subdirs of a
  two-module repo and installs them into the alice profile in one
  pass (current scaffold PR #75 `basecamp modules` walks root-level
  and immediate-sub flakes; confirm this includes our layout).
- [ ] Dev `#lgx` works end-to-end for this two-module topology. No
  `delivery_module` means no 2 s token-handshake race
  ([basecamp#169](https://github.com/logos-co/logos-basecamp/issues/169))
  — dev variant should be fine here.
- [ ] 3×3 minimax in C fits in ≤80 lines (9!/2 = 181 440 max leaves,
  trivial; no α-β pruning needed).
- [ ] Repo works as a `logos-tutorial` companion read: running the
  repo alongside the tutorial's Part 2 prose makes sense. Validate
  by walking Part 2 end-to-end with this repo as the example.

## MVP Scope

**In**
- Two-subdir monorepo at root: `logos-tictactoe-solo-ai/` and
  `logos-tictactoe-ui/`, plus repo-level `scaffold.toml`, `flake.nix`
  glue if scaffold emits one, `README.md`, `.github/workflows/`.
- Core: game state + win/draw detection + minimax AI via
  `newGame(vsAi)` arg. No multiplayer code anywhere.
- UI: QML-only, hotseat + "New Game vs AI" button, status line,
  win-line highlight.
- First commits are raw `lgs init` outputs per subdir (so reviewers
  can see the delta between scaffold template and our changes).
- `#lgx` + `#lgx-portable` builds per module, 6 individual artifacts
  in CI + release (same pattern as current repo).
- README: `lgs basecamp setup → install → launch alice` as the only
  documented path for users; link tutorial Part 2 for QML basics
  rather than rewriting.

**Out of MVP**
- Multiplayer, `delivery_module`, events, async RPC, `.rep` files,
  QRO, C++ backend.
- AI difficulty settings (MVP is always-perfect minimax).
- Undo / history / persistence / stats.
- Qt Widgets UI variant.

## Not Doing (and Why)

- **Multiplayer / `delivery_module`** — repo #2 (`logos-tictactoe-app`).
- **QML + C++ backend via `.rep`** — repo #2 (Part 3 analogue).
- **C core + QML+C++ UI across multiple flakes** — repo #3
  (`logos-tictactoe-app-module`, Part 1 + Part 3 combined).
- **Manual `lgpm install --file` workflow in docs** — dogfood
  constraint. Users install through `lgs basecamp` only.
- **Manual `nix build .#lgx` workflow in docs** — same. CI may use
  nix directly; user docs don't.
- **`onModuleEvent`-based UI refresh** — sync `callModule` + refresh
  after each click is enough for local-only play and keeps the Part
  2 teaching shape clean.
- **Migrating git history from current monorepo** — fresh `lgs init`
  lineage is cleaner than a filtered rewrite.

## Open Questions

- UI module naming: using `tictactoe_ui` (dir `logos-tictactoe-ui`)
  by default, mirroring tutorial's `calc_ui`. Flag if another name
  (`tictactoe_qml`, `tictactoe_solo_ai_ui`) is preferred.
- Repo-level `flake.nix`: does `lgs init` at repo root emit one, or
  is it only per-module? Determines whether the repo has a composite
  `#lgx`-all attr or only per-subdir. Verify during scaffold.
