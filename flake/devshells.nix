{
  inputs,
  lib,
  ...
}: {
  imports = [inputs.devshell.flakeModule];

  perSystem = {pkgs, ...}: {
    devshells.default = {
      packages = lib.attrsets.attrValues {
        inherit (pkgs.customPypi) keymap-drawer;
        inherit (pkgs) qmk;
      };
    };
  };
}
