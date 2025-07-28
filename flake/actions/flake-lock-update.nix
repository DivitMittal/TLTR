{
  common-permissions,
  common-actions,
  inputs,
  ...
}: {
  flake.actions-nix.workflows.".github/workflows/flake-lock-update.yml" = {
    on = {
      workflow_dispatch = {};
      schedule = [
        {
          cron = "0 0 * * 0"; # Every Sunday at midnight
        }
      ];
    };
    jobs.locking-flake = {
      permissions =
        common-permissions
        // {
          issues = "write";
          pull-requests = "write";
        };
      steps =
        common-actions
        ++ [
          inputs.actions-nix.lib.steps.DeterminateSystemsNixInstallerAction
          {
            name = "Update flake.lock";
            uses = "DeterminateSystems/update-flake-lock@main";
          }
        ];
    };
  };
}
