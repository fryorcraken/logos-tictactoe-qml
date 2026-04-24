{
  description = "QML UI for tic-tac-toe solo + AI";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
    # Tutorial Part 2 Step 5.2 "Option B" — sibling local-dev input. The
    # core module lives next to this UI in the same repo, so `path:` picks
    # up local edits directly (a `github:` URL would pull master or a stale
    # pin instead).
    tictactoe_solo_ai.url = "path:../logos-tictactoe-solo-ai";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosQmlModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;
    };
}
