{
  lib,
  stdenv,
  src,
  version ? src.shortRev or src.dirtyShortRev or "dev",
  pkg-config,
  wayland,
  wayland-protocols,
  wayland-scanner,
  mesa,
  libGL,
}:

stdenv.mkDerivation {
  pname = "hyprlax";
  inherit version src;

  nativeBuildInputs = [
    pkg-config
    wayland-scanner
  ];

  buildInputs = [
    wayland
    wayland-protocols
    mesa
    libGL
  ];

  # CI=1 avoids -march=native and -flto which break the Nix sandbox
  makeFlags = [
    "CI=1"
    "PREFIX=$(out)"
  ];

  meta = {
    description = "Buttery smooth parallax wallpaper daemon for Wayland compositors";
    homepage = "https://github.com/sandwichfarm/hyprlax";
    license = lib.licenses.mit;
    platforms = lib.platforms.linux;
    mainProgram = "hyprlax";
  };
}
