_: {
  flake.actions-nix.workflows.".github/workflows/keymap-drawer.yml" = {
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
      steps = [
        {
          name = "Checkout repo";
          uses = "actions/checkout@main";
          "with" = {
            fetch-depth = 1;
          };
        }
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
          name = "Run keymap-drawer for split keyboard";
          run = "keymap draw ./keymap-drawer/tltr.yml 1> assets/tltr.svg";
        }
        {
          name = "Run keymap-drawer for ANSI keyboard";
          run = "keymap draw ./keymap-drawer/tltr-ansi.yml 1> assets/tltr-ansi.svg";
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
}
