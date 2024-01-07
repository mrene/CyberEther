{ lib
, stdenv
, fetchFromGitHub
, meson
, ninja
, pkg-config
, cmake
, fmt_9
, soapysdr

# macOS deps
, spirv-cross
, glslang
, glfw
, darwin

}:

stdenv.mkDerivation rec {
  pname = "cyberether";
  version = "1.0.0-alpha1";

  src = fetchFromGitHub {
    owner = "luigifcruz";
    repo = "CyberEther";
    rev = "v${version}";
    hash = "sha256-AYqd58segYBhpVA/r1Kcr1riigvpI3SwKrZ3+jAbSt4=";
  };

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    cmake
  ];

  buildInputs = [
    soapysdr
    fmt_9

    glfw
    spirv-cross
    glslang
  ] ++  lib.optionals stdenv.isDarwin (with darwin.apple_sdk.frameworks; [ Foundation AVFoundation CoreFoundation CoreAudio AppKit Metal MetalKit AudioToolbox Cocoa ]);


  #mesonFlags = [
    #"-Dcpp_std=c++2a"
  #];

  meta = with lib; {
    description = "Radio: the final frontier";
    homepage = "https://github.com/luigifcruz/CyberEther";
    license = licenses.mit;
    maintainers = with maintainers; [ ];
    mainProgram = "cyberether";
    platforms = platforms.all;
  };
}
