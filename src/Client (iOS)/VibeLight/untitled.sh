#!/bin/sh

SOURCE_DIR="${PIPA_ROOT}/assets/Icons/Icon.png"


cd "${SOURCE_ROOT}/VibeLight/Assets.xcassets/AppIcon.appiconset/"

cp "${SOURCE_DIR}" "Icon_iPhone_29pt@2x.png"
cp "${SOURCE_DIR}" "Icon_iPhone_29pt@3x.png"

cp "${SOURCE_DIR}" "Icon_iPhone_40pt@2x.png"
cp "${SOURCE_DIR}" "Icon_iPhone_40pt@3x.png"

cp "${SOURCE_DIR}" "Icon_iPhone_60pt@2x.png"
cp "${SOURCE_DIR}" "Icon_iPhone_60pt@3x.png"

sips -z 58 58 "Icon_iPhone_29pt@2x.png"
sips -z 87 87 "Icon_iPhone_29pt@3x.png"

sips -z 80 80 "Icon_iPhone_40pt@2x.png"
sips -z 120 120 "Icon_iPhone_40pt@3x.png"

sips -z 120 120 "Icon_iPhone_60pt@2x.png"
sips -z 180 180 "Icon_iPhone_60pt@3x.png"
