# Home Manager module: runs the TLTR kanata layout through kanata-tray.
{inputs}: {
  config,
  lib,
  pkgs,
  ...
}: let
  inherit (lib) mkEnableOption mkIf mkOption types;
  inherit (pkgs.stdenvNoCC) hostPlatform;
  cfg = config.programs.tltr;
  tomlFormat = pkgs.formats.toml {};
in {
  options.programs.tltr = {
    enable = mkEnableOption "the TLTR layout via kanata & kanata-tray";

    package = mkOption {
      type = types.package;
      # cmd actions back the screen-lock & display-off keys on Linux & Windows
      default = pkgs.kanata.override {withCmd = true;};
      defaultText = lib.literalExpression "pkgs.kanata.override {withCmd = true;}";
      description = "kanata package; it needs cmd support for the screen-control keys.";
    };

    trayPackage = mkOption {
      type = types.package;
      default = pkgs.callPackage (inputs.kanata-tray + "/nix/package.nix") {self = inputs.kanata-tray;};
      defaultText = lib.literalExpression "kanata-tray built from TLTR's kanata-tray input";
      description = "kanata-tray package.";
    };

    configFile = mkOption {
      type = types.path;
      default = "${../kanata}/tltr.kbd";
      defaultText = lib.literalExpression ''"''${TLTR}/kanata/tltr.kbd"'';
      description = "kanata config the TLTR preset loads.";
    };

    extraArgs = mkOption {
      type = types.listOf types.str;
      default = ["--nodelay"];
      description = "Extra arguments passed to kanata.";
    };

    traySettings = mkOption {
      inherit (tomlFormat) type;
      default = {};
      description = ''
        Extra kanata-tray settings, merged over the TLTR defaults. See
        <https://github.com/rszyma/kanata-tray?tab=readme-ov-file#examples>.
      '';
    };
  };

  config = mkIf cfg.enable {
    home.packages =
      [cfg.package cfg.trayPackage]
      ++ lib.optional hostPlatform.isDarwin
      (pkgs.callPackage ./kanata-tray-daemon.nix {kanata-tray = cfg.trayPackage;});

    home.file.kanataTray = {
      source = tomlFormat.generate "kanata-tray.toml" (lib.recursiveUpdate {
          general.allow_concurrent_presets = false;
          defaults = {
            kanata_executable = "${cfg.package}/bin/kanata";
            tcp_port = 5830;
          };
          presets.TLTR = {
            kanata_config = "${cfg.configFile}";
            autorun = true;
            extra_args = cfg.extraArgs;
          };
        }
        cfg.traySettings);
      target =
        (
          if hostPlatform.isDarwin
          then "Library/Application Support"
          else lib.removePrefix "${config.home.homeDirectory}/" config.xdg.configHome
        )
        + "/kanata-tray/kanata-tray.toml";
    };
  };
}
