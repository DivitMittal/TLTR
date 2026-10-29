_: {
  flake.actions-nix.workflows.".github/workflows/keymap-drawer.yml" = {
    on = rec {
      push = {
        branches = ["master"];
        paths = [
          "keymap-drawer/**"
          ".github/workflows/keymap-drawer.yml"
        ];
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
          # Commits that already include the regenerated SVGs leave nothing to
          # commit, and a bare `git commit` exits 1 on that. PRs only render.
          name = "Push to repo";
          "if" = "github.event_name != 'pull_request'";
          run = ''
            git add assets/tltr.svg assets/tltr-ansi.svg
            if git diff --cached --quiet; then
              echo "Keymap drawings already up to date."
              exit 0
            fi
            git config --global user.name "GitHub Actions Bot"
            git config --global user.email bot@github.com
            git commit -m "chore: update keymap-drawer assets"
            git push origin HEAD:master
          '';
          env = {
            GITHUB_TOKEN = "\${{ secrets.GITHUB_TOKEN }}";
          };
        }
      ];
    };
  };
}
