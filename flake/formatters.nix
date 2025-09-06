{inputs, ...}: {
  imports = [inputs.treefmt-nix.flakeModule];
  perSystem.treefmt = {
    projectRootFile = "flake.nix";
    settings.global = {
      excludes = [
        ".github/*"
      ];
    };

    flakeCheck = false;

    programs = {
      #typos.enable = true;
      ## Nix
      alejandra.enable = true;
      deadnix.enable = true;
      statix.enable = true;
      ## C
      clang-format.enable = true;
      ## JSON
      prettier.enable = true;
    };
  };
}
