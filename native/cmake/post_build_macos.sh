#!/bin/sh
set -e

SRC="$1"
BUILD_DIR="$2"
UNITY_DIR="$3"

TS=$(date +%Y%m%d_%H%M%S)

mkdir -p "$BUILD_DIR"
mkdir -p "$UNITY_DIR"

DST_BUILD="$BUILD_DIR/AudioPluginBRTUnity_${TS}.bundle"
DST_UNITY="$UNITY_DIR/AudioPluginBRTUnity.bundle"

# copy to archive
cp -R "$SRC" "$DST_BUILD"

# deploy to Unity (real folder)
rm -rf "$DST_UNITY"
cp -R "$DST_BUILD" "$DST_UNITY"

# IMPORTANT: sign AFTER final location
codesign --force --deep --sign - "$DST_UNITY"

echo "Built: $DST_BUILD"
echo "Deployed + signed: $DST_UNITY"
