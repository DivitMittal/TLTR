{
  common-actions,
  inputs,
  ...
}: {
  flake.actions-nix.workflows.".github/workflows/kanata-check.yml" = {
    on = rec {
      push = {
        branches = ["master"];
        paths = [
          "kanata/**"
          "flake.lock"
          "flake/kanata.nix"
          "flake/nixpkgs.nix"
          ".github/workflows/kanata-check.yml"
        ];
      };
      pull_request = push;
      workflow_dispatch = {};
    };
    jobs.kanata-check = {
      # kanata only parses the (platform ...) blocks matching the host, so
      # validate on both Linux and macOS.
      strategy = {
        fail-fast = false;
        matrix.os = ["ubuntu-latest" "macos-latest"];
      };
      runs-on = "\${{ matrix.os }}";
      permissions.contents = "read";
      steps =
        common-actions
        ++ [
          inputs.actions-nix.lib.steps.DeterminateSystemsNixInstallerAction
          {
            name = "Magic Nix Cache(Use GitHub Actions Cache)";
            uses = "DeterminateSystems/magic-nix-cache-action@main";
          }
          {
            name = "Validate kanata config";
            run = ''nix build -L ".#checks.$(nix eval --impure --raw --expr builtins.currentSystem).kanata-config"'';
          }
        ];
    };
  };
}
