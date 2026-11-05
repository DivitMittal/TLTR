{common-actions, ...}: {
  flake.actions-nix.workflows.".github/workflows/hardware-check.yml" = {
    on = rec {
      push = {
        branches = ["master"];
        paths = [
          "hardware/**"
          ".github/workflows/hardware-check.yml"
        ];
      };
      pull_request = push;
      workflow_dispatch = {};
    };
    jobs.hardware-check = {
      # Official KiCad image: kicad-cli plus the pcbnew Python module the
      # generators need. nixpkgs' KiCad lags behind the KiCad 10 file format.
      container = {
        image = "kicad/kicad:10.0";
        options = "--user root";
      };
      permissions.contents = "read";
      env.KICAD_CONFIG_HOME = "/tmp/kicad-config";
      steps =
        common-actions
        ++ [
          {
            # Without global library tables ERC reports every stock library as missing.
            name = "Seed KiCad library tables";
            run = ''
              mkdir -p "$KICAD_CONFIG_HOME/10.0"
              for t in fp-lib-table sym-lib-table; do
                cp "/usr/share/kicad/template/$t" "$KICAD_CONFIG_HOME/"
                cp "/usr/share/kicad/template/$t" "$KICAD_CONFIG_HOME/10.0/"
              done
            '';
          }
          {
            # generate.py exits non-zero on any ERC, DRC or schematic-parity violation.
            name = "Regenerate boards, run ERC and DRC";
            run = ''
              for gen in hardware/*/generate.py; do
                echo "::group::$gen"
                python3 "$gen"
                echo "::endgroup::"
              done
            '';
          }
          {
            # The generators are the source of truth; committed KiCad files must match them.
            name = "Check committed KiCad files are up to date";
            run = ''
              git config --global --add safe.directory "$GITHUB_WORKSPACE"
              git diff --exit-code -- 'hardware/*.kicad_*' 'hardware/**/*.kicad_*' 'hardware/**/mechanical.json'
            '';
          }
          {
            name = "Upload ERC/DRC reports";
            "if" = "always()";
            uses = "actions/upload-artifact@v4";
            "with" = {
              name = "hardware-reports";
              path = ".validation/";
              if-no-files-found = "ignore";
            };
          }
        ];
    };
  };
}
