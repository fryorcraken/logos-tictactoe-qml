{
  description = "Tic-tac-toe core module with minimax AI";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;

      # Workaround: module-builder's `vendor_path` external-library handler
      # only copies `lib*` files from the vendor dir — it does NOT run the
      # library's build_command (unlike the flake-input path in
      # mkExternalLib.nix). Without this, the plugin links against undefined
      # `tictactoe_*` symbols and crashes at first call inside basecamp.
      # Compile libtictactoe.so in-sandbox so `.#lgx` is self-contained.
      # Remove once module-builder honours build_command for vendor_path:
      # https://github.com/logos-co/logos-module-builder/issues/83
      preConfigure = ''
        if [ -f lib/libtictactoe.c ] && [ ! -f lib/libtictactoe.so ]; then
          echo "Compiling vendored libtictactoe.so..."
          (cd lib && gcc -shared -fPIC -o libtictactoe.so libtictactoe.c)
        fi
      '';
    };
}
