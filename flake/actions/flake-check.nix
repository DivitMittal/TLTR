{
  common-on,
  common-permissions,
  common-actions,
  inputs,
  ...
}: {
  flake.actions-nix.workflows.".github/workflows/flake-check.yml" = {
    on = common-on;
    jobs.checking-flake = {
      permissions = common-permissions;
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
}