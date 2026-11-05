{
  common-actions,
  inputs,
  ...
}: {
  flake.actions-nix.workflows.".github/workflows/pedal-tests.yml" = {
    on = rec {
      push = {
        branches = ["master"];
        paths = [
          "pedals/**"
          "flake.nix"
          "flake.lock"
          "flake/**"
        ];
      };
      pull_request = push;
      workflow_dispatch = {};
    };
    jobs.pedal-tests = {
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
            name = "Run pedal gesture tests";
            run = "nix build .#checks.x86_64-linux.pedal-gesture-tests --print-build-logs";
          }
        ];
    };
  };
}
