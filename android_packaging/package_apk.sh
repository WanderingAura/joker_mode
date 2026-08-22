#!/usr/bin/env bash
# Packages dist/libsociety.so + AndroidManifest.xml + assets/ into a signed,
# installable debug APK. Requires: aapt2, zipalign, apksigner, keytool, zip
# (all on PATH), and an already-built dist/libsociety.so (debug-android /
# release-android preset).

set -euo pipefail

CURRENT_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$CURRENT_SCRIPT_DIR")"

cd "$CURRENT_SCRIPT_DIR"

ANDROID_HOME="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}}"
PLATFORM_VERSION="android-34"
ANDROID_JAR="$ANDROID_HOME/platforms/$PLATFORM_VERSION/android.jar"

ABI="arm64-v8a"
SO_NAME="libsociety.so"
SO_PATH="$PROJECT_ROOT/dist/$SO_NAME"

MANIFEST="AndroidManifest.xml"
ASSETS_STAGING="apk_assets"
BUILD_DIR="build"
KEYSTORE="debug.keystore"
KEYSTORE_PASS="android"
KEY_ALIAS="androiddebugkey"

OUT_APK="society-debug.apk"

if [ ! -f "$ANDROID_JAR" ]; then
    echo "error: android.jar not found at $ANDROID_JAR (check ANDROID_HOME / installed platforms)" >&2
    exit 1
fi

if [ ! -f "$SO_PATH" ]; then
    echo "error: $SO_PATH not found, build the debug-android/release-android preset first" >&2
    exit 1
fi

echo "===================================="
echo "Staging assets..."
echo "===================================="
mkdir -p "$ASSETS_STAGING"
ln -sfn "$PROJECT_ROOT/assets" "$ASSETS_STAGING/assets"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/lib/$ABI"
cp "$SO_PATH" "$BUILD_DIR/lib/$ABI/"

echo "===================================="
echo "Linking manifest + assets (aapt2 link)..."
echo "===================================="
aapt2 link \
    -I "$ANDROID_JAR" \
    --manifest "$MANIFEST" \
    -A "$ASSETS_STAGING" \
    -o "$BUILD_DIR/base.apk"

echo "===================================="
echo "Adding native library..."
echo "===================================="
cp "$BUILD_DIR/base.apk" "$BUILD_DIR/unaligned.apk"
(cd "$BUILD_DIR" && zip -Xr unaligned.apk lib)

echo "===================================="
echo "Aligning..."
echo "===================================="
zipalign -f -p 4 "$BUILD_DIR/unaligned.apk" "$BUILD_DIR/aligned.apk"

echo "===================================="
echo "Signing..."
echo "===================================="
if [ ! -f "$KEYSTORE" ]; then
    echo "No debug keystore found at $KEYSTORE, generating one..."
    keytool -genkeypair -v \
        -keystore "$KEYSTORE" -storepass "$KEYSTORE_PASS" \
        -alias "$KEY_ALIAS" -keypass "$KEYSTORE_PASS" \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US"
fi

apksigner sign \
    --ks "$KEYSTORE" --ks-pass "pass:$KEYSTORE_PASS" --key-pass "pass:$KEYSTORE_PASS" \
    --out "$OUT_APK" \
    "$BUILD_DIR/aligned.apk"

echo "===================================="
echo "Successfully built $OUT_APK"
echo "===================================="
echo "Install with: adb install -r $CURRENT_SCRIPT_DIR/$OUT_APK"
