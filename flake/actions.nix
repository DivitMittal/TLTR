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
      permissions = {
        contents = "write";
        id-token = "write";
      };
      common-actions = [
        {
          name = "Checkout repo";
          uses = "actions/checkout@main";
          "with" = {
            fetch-depth = 1;
          };
        }
      ];
    in {
      ".github/workflows/flake-check.yml" = {
        inherit on;
        jobs.checking-flake = {
          inherit permissions;
          steps =
            common-actions
            ++ [
              inputs.actions-nix.lib.steps.DeterminateSystemsNixInstallerAction
              {
                name = "Magic Nix Cache(Use Github Actions Cache)";
                uses = "DeterminateSystems/magic-nix-cache-action@main";
              }
              {
                name = "Run nix flake check";
                run = "nix flake check --impure --all-systems --no-build";
              }
            ];
        };
      };
      ".github/workflows/keymap-drawer.yml" = {
        on = {
          push = {
            branches = ["master"];
            paths = ["keymap-drawer/**"];
          };
          workflow_dispatch = {};
        };
        jobs.keymap-drawer = {
          permissions = {
            contents = "write";
          };
          steps =
            common-actions
            ++ [
              {
                name = "Install uv with caching";
                uses = "astral-sh/setup-uv@main";
                "with" = {
                  cache = "true";
                };
              }
              {
                name = "Install keymap-drawer";
                run = "uv tool install keymap-drawer";
              }
              {
                name = "Run keymap-drawer";
                run = "keymap draw ./keymap-drawer/tltr.yml 1> assets/tltr.svg";
              }
            ];
        };
      };
    };
  };
}