_: {
  perSystem = {pkgs, ...}: {
    # Parses the kanata config the way kanata would at startup. The config uses
    # (cmd ...) actions, which nixpkgs' default kanata build rejects. Only the
    # host platform's (platform ...) blocks get validated.
    checks.kanata-config =
      pkgs.runCommand "kanata-config-check" {
        nativeBuildInputs = [(pkgs.kanata.override {withCmd = true;})];
      } ''
        cd ${../kanata}
        kanata --check --cfg tltr.kbd
        touch $out
      '';
  };
}
