#!/usr/bin/env bash
# Build SurgeXXL for MPC OS Gen1 (the Force: armv7-a hard-float, glibc <= 2.31 so it also loads on MPC OS 2.x).
#   ./build.sh                 the plugin folder in build/package/
#   ./build.sh --skin-only     just the skin (build/skin/), to look at with test/preview.sh
#   ./build.sh --sdk           the MPC plugin SDK folder (MPC OS 3.10.1+, 32- and 64-bit MPCs) in build/sdk/
# Needs: python3 + Pillow + cairosvg, cmake + ninja, a host gcc (for shadow_art), and Zig (pip install ziglang).
# No Docker. Output: build/package/L'Cronx - VST - SurgeXXL/ (skin, surgexxl.so and Surge's data folder).
# Set SXXL_WT_PICTURES=0 to build without the 761 wavetable pictures (a lighter skin, if MPC finds it too heavy).
set -euo pipefail
cd "$(dirname "$0")"
MPC_VST="${MPC_VST:-$PWD/third_party/mpc-vst-plugins}"
NAME="L'Cronx - VST - SurgeXXL"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 2)}"

python3 tools/gen_params.py
python3 tools/panel.py --check   # layout.conf matches tools/panel.py and qlinks.json

# ---- the skin
mkdir -p build/host build/gen
[ -x build/host/shadow_art ] || gcc -O2 -w -I"$MPC_VST/tools/vendor/force-shadow/tools" -x c -o build/host/shadow_art "$MPC_VST/tools/shadow_art.c" -lm
rm -rf build/skin
SHADOW_TITLE_FONT="$PWD/art/fonts/TitilliumWeb-SemiBold.ttf" SHADOW_ART="$PWD/build/host/shadow_art" \
  python3 "$MPC_VST/tools/gen_vst.py" vst.json 2>&1 | grep -v -e "TOUCH" -e "EDGE" || true   # panel_art places every control
[ -f build/params.h ] && [ -d "build/skin/$NAME/Plugin Skins" ]
python3 tools/panel_art.py "build/skin/$NAME/Plugin Skins"
python3 "$MPC_VST/tools/skin_check.py" "build/skin/$NAME" || true
[ "${1:-}" = --skin-only ] && exit 0

# ---- --sdk: the plugin folder of Akai's MPC plugin SDK (mpc-plugin-builder-sdk docs/plugin-folder-format.md), for
# MPC OS 3.10.1+ "External Plugin Support": copy it into <drive>/VST2 Plugins/. SDK_ARCHS picks the libraries:
# arm64 (MPC Live III, MPC XL), arm32 (the other MPCs and the Force), or both (the default: the SDK wants both).
if [ "${1:-}" = --sdk ]; then
  mkdir -p build/gen-sdk
  sed 's|^#define MODULE_SUBDIR .*|#define MODULE_SUBDIR "../Content"|' build/params.h > build/gen-sdk/params.h.new
  cmp -s build/gen-sdk/params.h.new build/gen-sdk/params.h 2>/dev/null || mv build/gen-sdk/params.h.new build/gen-sdk/params.h
  python3 tools/wrap_chunk.py "$MPC_VST" build/gen-sdk/vst2_wrap.c.new
  cmp -s build/gen-sdk/vst2_wrap.c.new build/gen-sdk/vst2_wrap.c 2>/dev/null || mv build/gen-sdk/vst2_wrap.c.new build/gen-sdk/vst2_wrap.c
  rm -f build/gen-sdk/*.new
  PKG="build/sdk/$NAME"
  rm -rf build/sdk && mkdir -p "$PKG/VST"
  FILES=""
  for arch in ${SDK_ARCHS:-arm32 arm64}; do
    case "$arch" in
      arm64) tc=tools/aarch64-zig.cmake ;;
      arm32) tc=tools/armv7-zig.cmake ;;
      *) echo "SDK_ARCHS: arm64, arm32 or both, not '$arch'" >&2; exit 1 ;;
    esac
    cmake -S . -B "build/sdk-$arch" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$PWD/$tc" \
      -DMPC_VST="$MPC_VST" -DSXXL_GEN="$PWD/build/gen-sdk" ${SXXL_CMAKE_ARGS:-} > "build/sdk-$arch-configure.log"
    ninja -C "build/sdk-$arch" -j"$JOBS" surgexxl
    cp "build/sdk-$arch/surgexxl.so" "$PKG/VST/surgexxl-$arch.so"
    FILES="$FILES  \"file${arch#arm}\": \"VST/surgexxl-$arch.so\",
"
  done
  cp -r "build/skin/$NAME/Plugin Skins" "$PKG/"
  tools/stage_data.sh "$PKG"
  mv "$PKG/data" "$PKG/Content"
  VER="$(sed -n 's/^project(SurgeXXL VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
  cat > "$PKG/plugin.json" <<JSON
{
  "name": "SurgeXXL",
  "manufacturer": "L'Cronx",
  "version": "$VER",
  "format": "vst2",
  "kind": "instrument",
  "uid": "AnSx",
  "category": "Synth",
  "numInputs": 0,
  "numOutputs": 2,
$FILES  "description": "Surge XT 1.3.4's sound engine as an MPC instrument, with every Surge factory and third-party patch and wavetable.",
  "license": "GPL-3.0-or-later"
}
JSON
  cat > "$PKG/version.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<plugincontent version="1.0">
    <version>$VER.0</version>
    <uiversion>0.1.0.0</uiversion>
</plugincontent>
XML
  echo "-> $PKG"
  exit 0
fi

# ---- the library: Surge's engine and the plugin layer, with the framework's wrapper (state chunk enlarged)
cp build/params.h build/gen/params.h
python3 tools/wrap_chunk.py "$MPC_VST" build/gen/vst2_wrap.c
cmake -S . -B build/arm -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$PWD/tools/armv7-zig.cmake" \
  -DMPC_VST="$MPC_VST" > build/arm-configure.log
ninja -C build/arm -j"$JOBS" surgexxl

# ---- the plugin folder
PKG="build/package/$NAME"
rm -rf build/package && mkdir -p "$PKG"
cp -r "build/skin/$NAME/." "$PKG/"
cp build/arm/surgexxl.so "$PKG/"
tools/stage_data.sh "$PKG"
echo "-> $PKG"
