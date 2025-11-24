{
  inputs,
  customLib,
  ...
}: {
  imports =
    (customLib.scanPaths ./.)
    ++ [
      inputs.actions-nix.flakeModules.default
    ];

  _module.args = {
    common-on = rec {
      push = {
        branches = ["master"];
        paths = [
          "flake.nix"
          "flake.lock"
          "flake/**"
        ];
      };
      pull_request = push;
      workflow_dispatch = {};
    };
    common-permissions = {
      contents = "write";
      id-token = "write";
    };
    common-actions = [
      {
        name = "Checkout repo";
        uses = "actions/checkout@main";
        "with" = {
          fetch-depth = 1;
          persist-credentials = false;
        };
      }
    ];
  };

  flake.actions-nix = {
    pre-commit.enable = true;
    defaultValues = {
      jobs = {
        runs-on = "ubuntu-latest";
        timeout-minutes = 30;
      };
    };
  };
}
