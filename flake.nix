{
  description = "TLTR flake";
  outputs = inputs: let
    inherit (inputs.flake-parts.lib) mkFlake;
    specialArgs.customLib = inputs.OS-nixCfg.lib;
  in
    mkFlake {inherit inputs specialArgs;} ({
      inputs,
      lib,
      ...
    }: {
      systems = builtins.import inputs.systems;
      perSystem = {system, ...}: {
        _module.args.pkgs = builtins.import inputs.nixpkgs {
          inherit system;
          config = let
            inherit (lib) mkDefault;
          in {
            allowUnfree = mkDefault true;
            allowBroken = mkDefault false;
            allowUnsupportedSystem = mkDefault false;
            checkMeta = mkDefault false;
            warnUndeclaredOptions = mkDefault true;
          };
          overlays = lib.attrsets.attrValues {
            inherit
              (inputs.OS-nixCfg.overlays)
              default
              ;
          };
        };
      };
      imports = [./flake];
    });

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    systems.url = "github:nix-systems/default";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    git-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    actions-nix = {
      url = "github:nialov/actions.nix";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        flake-parts.follows = "flake-parts";
        git-hooks.follows = "git-hooks";
      };
    };
    devshell = {
      url = "github:numtide/devshell";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    OS-nixCfg = {
      url = "github:DivitMittal/OS-nixCfg";
      inputs = {
        systems.follows = "systems";
        nixpkgs.follows = "nixpkgs";
        flake-parts.follows = "flake-parts";
        treefmt-nix.follows = "treefmt-nix";
        actions-nix.follows = "actions-nix";
        git-hooks.follows = "git-hooks";
      };
    };
  };
}
