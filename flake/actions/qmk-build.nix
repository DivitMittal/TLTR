{common-actions, ...}: {
  flake.actions-nix.workflows.".github/workflows/qmk-build.yml" = {
    on = rec {
      push = {
        branches = ["master"];
        paths = [
          "qmk/**"
          ".github/workflows/qmk-build.yml"
        ];
      };
      pull_request = push;
      workflow_dispatch = {};
    };
    jobs.qmk-build = {
      # Official QMK image, ships the ARM toolchain and qmk CLI.
      container = "ghcr.io/qmk/qmk_cli:latest";
      strategy = {
        fail-fast = false;
        matrix.half = ["left" "right"];
      };
      permissions.contents = "read";
      env.QMK_HOME = "/qmk_firmware";
      steps =
        common-actions
        ++ [
          {
            name = "Fetch QMK firmware";
            run = ''git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/qmk/qmk_firmware.git "$QMK_HOME"'';
          }
          {
            name = "Link TLTR keyboard into QMK";
            run = ''ln -s "$GITHUB_WORKSPACE/qmk/piantor_tltr" "$QMK_HOME/keyboards/beekeeb/piantor_tltr"'';
          }
          {
            name = "Compile \${{ matrix.half }} half";
            run = ''
              cd "$QMK_HOME"
              qmk compile -kb beekeeb/piantor_tltr -km tltr \
                -e TLTR_HALF=''${{ matrix.half }} \
                -e TARGET=beekeeb_piantor_tltr_tltr_''${{ matrix.half }}
              mkdir -p "$GITHUB_WORKSPACE/firmware"
              cp .build/*.uf2 "$GITHUB_WORKSPACE/firmware/"
            '';
          }
          {
            name = "Upload firmware";
            uses = "actions/upload-artifact@v4";
            "with" = {
              name = "tltr-\${{ matrix.half }}";
              path = "firmware/*.uf2";
              if-no-files-found = "error";
            };
          }
        ];
    };
  };
}
