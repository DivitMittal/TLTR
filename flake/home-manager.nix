{
  inputs,
  self,
  ...
}: {
  flake.homeManagerModules = {
    tltr = import ../nix/hm-module.nix {inherit inputs;};
    default = self.homeManagerModules.tltr;
  };

  perSystem = {
    lib,
    pkgs,
    ...
  }: let
    kanata-tray = pkgs.callPackage (inputs.kanata-tray + "/nix/package.nix") {self = inputs.kanata-tray;};
  in {
    packages =
      {inherit kanata-tray;}
      // lib.optionalAttrs pkgs.stdenvNoCC.hostPlatform.isDarwin {
        kanata-tray-daemon = pkgs.callPackage ../nix/kanata-tray-daemon.nix {inherit kanata-tray;};
      };
  };
}
