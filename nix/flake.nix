{
  description = "Description for the project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # Use meson from staging in order to fix https://github.com/luigifcruz/CyberEther/issues/38
    # Tracking PR: https://nixpk.gs/pr-tracker.html?pr=268583
    nixpkgs-staging.url = "github:NixOS/nixpkgs/staging-next";
  };

  outputs = inputs@{ flake-parts, nixpkgs-staging, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        # To import a flake module
        # 1. Add foo to inputs
        # 2. Add foo as a parameter to the outputs function
        # 3. Add here: foo.flakeModule

      ];
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: 
      let 
        pkgsStaging = import nixpkgs-staging { inherit system; };
      in {
        # Per-system attributes can be defined here. The self' and inputs'
        # module parameters provide easy access to attributes of the same
        # system.

        # Equivalent to  inputs'.nixpkgs.legacyPackages.hello;
        packages.default = pkgs.callPackage ./default.nix { meson = pkgsStaging.meson; };
      };
      flake = {
        # The usual flake attributes can be defined here, including system-
        # agnostic ones like nixosModule and system-enumerating ones, although
        # those are more easily expressed in perSystem.

      };
    };
}
