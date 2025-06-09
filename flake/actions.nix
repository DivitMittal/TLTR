{inputs, ...}: {
  imports = [inputs.actions-nix.flakeModules.default];

  flake.actions-nix = {
    pre-commit.enable = true;
    defaults = {
      jobs = {
        runs-on = "ubuntu-latest";
        timeout-minutes = 30;
      };
    };

    workflows = let
      on = {
        push = {
          branches = ["master"];
          paths-ignore = [
            "**/*.md"
            ".github/**"
          ];
        };
        pull_request = {
          branches = ["master"];
        };
        workflow_dispatch = {};
      };
      common-actions = [
        {
          name = "Checkout repo";
          uses = "actions/checkout@main";
          "with" = {
            fetch-depth = 1;
          };
        }
        inputs.actions-nix.lib.steps.DeterminateSystemsNixInstallerAction
      ];
    in {
      ".github/workflows/flake-check.yml" = {
        inherit on;
        jobs.checking-flake = {
          steps =
            common-actions
            ++ [
              {
                name = "Run nix flake check";
                run = "nix flake check --impure --all-systems --no-build";
              }
            ];
        };
      };
    };
  };
}