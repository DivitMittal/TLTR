{inputs, ...}: {
  imports = [inputs.treefmt-nix.flakeModule];
  perSystem.treefmt = {
    projectRootFile = "flake.nix";

    flakeCheck = false;

    programs = {
      #typos.enable = true;
      ## Nix
      alejandra.enable = true;
      deadnix.enable = true;
      statix.enable = true;
      ## JSON
      prettier.enable = true;
    };
  };
}
