{inputs, ...}: {
  perSystem = {system, ...}: {
    # nixpkgs-unstable no longer evaluates on Intel Macs, so pin them to the
    # last nixpkgs branch that still supports x86_64-darwin. Other systems keep
    # flake-parts' default (inputs.nixpkgs).
    _module.args.pkgs =
      if system == "x86_64-darwin"
      then inputs.nixpkgs-x86_64-darwin.legacyPackages.${system}
      else inputs.nixpkgs.legacyPackages.${system};
  };
}
