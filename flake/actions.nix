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
      on = rec {
        push = {
          branches = ["master"];
          paths = [
            "flake.nix"
            "flake.lock"
            "assets/**"
            "flake/**"
          ];
        };
        pull_request = push;
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
                name = "Magic Nix Cache(Use GitHub Actions Cache)";
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
        on = rec {
          push = {
            branches = ["master"];
            paths = ["keymap-drawer/**"];
          };
          pull_request = push;
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
                  enable-cache = "true";
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
              {
                name = "Push to repo";
                run = ''
                  git config --global user.name "GitHub Actions Bot"
                  git config --global user.email bot@github.com
                  git add .
                  git commit -m "chore: update keymap-drawer assets"
                  git push origin master
                '';
                env = {
                  GITHUB_TOKEN = "\${{ secrets.GITHUB_TOKEN }}";
                };
              }
            ];
        };
      };
    };
  };
}
