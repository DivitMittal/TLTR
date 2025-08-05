{
  inputs,
  lib,
  ...
}: {
  imports = [inputs.devshell.flakeModule];

  perSystem = {
    pkgs,
    config,
    ...
  }: {
    devshells.default = {
      devshell = rec {
        name = "TLTR";
        motd = "{202}Welcome to {91}${name} {202}devshell!{reset} \n $(menu)";
        startup = {
          git-hooks.text = ''
            ${config.pre-commit.installationScript}
          '';
        };
        packages = lib.attrsets.attrValues {
          inherit
            (pkgs)
            ### LSPs & Formatters
            nixd
            alejandra
            ## qmk cli
            qmk
            ;
          keymap-drawer = pkgs.writeScriptBin "keymap" ''
            exec ${pkgs.uv}/bin/uv tool run --from keymap-drawer keymap "$@"
          '';
        };
      };
    };
  };
}
