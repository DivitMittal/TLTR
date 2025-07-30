{
  description = "TLTR's flake";
  outputs = inputs: let
    inherit (inputs.flake-parts.lib) mkFlake;
    specialArgs.customLib = inputs.OS-nixCfg.lib;
  in
    mkFlake {inherit inputs specialArgs;} ({inputs, ...}: {
      systems = builtins.import inputs.systems;
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
      flake = false;
    };
  };
}
