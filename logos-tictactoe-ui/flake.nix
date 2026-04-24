{
  description = "QML UI for tic-tac-toe solo + AI";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
    # Deviation from tutorial: tutorial uses
    #   calc_module.url = "github:logos-co/logos-tutorial?dir=logos-calc-module";
    # Tutorial's core module lives in a separate public repo that's already
    # been pushed. Ours is a sibling directory in the same repo being built
    # alongside this UI, so `path:` is the correct reference — a github URL
    # would pull master (or a stale pin) instead of picking up local edits
    # to the sibling core module.
    tictactoe_solo_ai.url = "path:../logos-tictactoe-solo-ai";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosQmlModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;
    };
}
