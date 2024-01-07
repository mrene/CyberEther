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
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: 
      let 
        pkgsStaging = import nixpkgs-staging { inherit system; };
      in {
        packages.default = pkgs.callPackage ./default.nix {
          soapysdr = pkgs.soapysdr.override (old: {
            extraPackages = [ pkgs.soapyuhd ];
          });
          inherit (pkgsStaging) meson;
          inherit (pkgs.darwin.apple_sdk.frameworks) Foundation AVFoundation CoreAudio AppKit Metal MetalKit AudioToolbox Cocoa;
        };      
      };
    };
}
