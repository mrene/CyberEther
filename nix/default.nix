{ lib
, stdenv
, fetchFromGitHub
, meson
, ninja
, pkg-config
, cmake
, fmt_9
, soapysdr
, gst_all_1
, spirv-cross
, glslang
, glfw

# Linux deps
, vulkan-validation-layers
, vulkan-headers

# macOS deps
, Foundation
, AVFoundation
, CoreAudio
, AppKit
, Metal
, MetalKit
, AudioToolbox
, Cocoa
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

    gst_all_1.gstreamer
    gst_all_1.gst-plugins-base
    gst_all_1.gst-plugins-good
    gst_all_1.gst-libav
  ] ++ lib.optionals stdenv.isLinux [
    vulkan-validation-layers
    vulkan-headers
  ] ++ lib.optionals stdenv.isDarwin [ 
    Foundation 
    AVFoundation
    CoreAudio
    AppKit
    Metal
    MetalKit 
    AudioToolbox
    Cocoa
  ];

  meta = with lib; {
    description = "Radio: the final frontier";
    homepage = "https://github.com/luigifcruz/CyberEther";
    license = licenses.mit;
    maintainers = with maintainers; [ ];
    mainProgram = "cyberether";
    platforms = platforms.all;
  };
}
