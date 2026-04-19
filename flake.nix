{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems =
        f: nixpkgs.lib.genAttrs supportedSystems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: {
        default = pkgs.callPackage ./nix/package.nix { src = self; };
      });

      checks = forAllSystems (pkgs: {
        default = pkgs.callPackage ./nix/package.nix { src = self; };
      });

      homeModules.default = import ./nix/hm-module.nix;
    };
}
