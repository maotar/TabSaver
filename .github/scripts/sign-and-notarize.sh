#!/bin/bash
set -e

echo "======================================"
echo "TabSaver Code Signing & Notarization"
echo "======================================"
echo ""

# Check required environment variables
if [ -z "$DEVELOPER_ID_CERTIFICATE" ] || [ -z "$DEVELOPER_ID_CERTIFICATE_PASSWORD" ]; then
    echo "Error: Certificate environment variables not set"
    exit 1
fi

if [ -z "$APPLE_ID" ] || [ -z "$APPLE_ID_PASSWORD" ] || [ -z "$APPLE_TEAM_ID" ]; then
    echo "Error: Apple ID environment variables not set"
    exit 1
fi

# Get version from CMakeLists.txt
VERSION=$(grep "project(TabVST VERSION" CMakeLists.txt | sed 's/.*VERSION \([0-9.]*\).*/\1/')
echo "Version: $VERSION"
echo ""

# Setup paths
ARTIFACTS_DIR="artifacts"
VST3_PATH="$ARTIFACTS_DIR/TabSaver.vst3"
AU_PATH="$ARTIFACTS_DIR/TabSaver.component"
DMG_NAME="TabSaver-macOS.dmg"
ENTITLEMENTS="macos/entitlements.plist"

# Create temporary keychain
KEYCHAIN_PATH="$RUNNER_TEMP/build.keychain"
KEYCHAIN_PASSWORD=$(openssl rand -base64 32)

echo "Setting up temporary keychain..."
security create-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN_PATH"
security set-keychain-settings -lut 21600 "$KEYCHAIN_PATH"
security unlock-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN_PATH"

# Decode and import certificate
echo "Importing Developer ID certificate..."
echo "$DEVELOPER_ID_CERTIFICATE" | base64 --decode > certificate.p12
security import certificate.p12 -k "$KEYCHAIN_PATH" -P "$DEVELOPER_ID_CERTIFICATE_PASSWORD" -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$KEYCHAIN_PASSWORD" "$KEYCHAIN_PATH"

# Add keychain to search list
security list-keychains -d user -s "$KEYCHAIN_PATH" $(security list-keychains -d user | sed s/\"//g)

# Find signing identity
IDENTITY=$(security find-identity -v -p codesigning "$KEYCHAIN_PATH" | grep "Developer ID Application" | head -1 | grep -o '".*"' | sed 's/"//g')
echo "Signing identity: $IDENTITY"
echo ""

# Sign VST3
if [ -d "$VST3_PATH" ]; then
    echo "Signing VST3 plugin..."
    codesign --force --options runtime --deep --timestamp \
        --entitlements "$ENTITLEMENTS" \
        --sign "$IDENTITY" \
        "$VST3_PATH"
    codesign -v "$VST3_PATH"
    echo "  ✓ VST3 signed successfully"
else
    echo "  ⚠ VST3 not found, skipping..."
fi

echo ""

# Sign AU
if [ -d "$AU_PATH" ]; then
    echo "Signing AU plugin..."
    codesign --force --options runtime --deep --timestamp \
        --entitlements "$ENTITLEMENTS" \
        --sign "$IDENTITY" \
        "$AU_PATH"
    codesign -v "$AU_PATH"
    echo "  ✓ AU signed successfully"
else
    echo "  ⚠ AU not found, skipping..."
fi

echo ""
echo "Creating DMG installer..."

# Create temporary directory for DMG contents
DMG_TEMP="$RUNNER_TEMP/dmg"
mkdir -p "$DMG_TEMP"

# Copy signed plugins
if [ -d "$VST3_PATH" ]; then
    cp -r "$VST3_PATH" "$DMG_TEMP/"
fi

if [ -d "$AU_PATH" ]; then
    cp -r "$AU_PATH" "$DMG_TEMP/"
fi

# Copy installation instructions
cp README-MACOS.txt "$DMG_TEMP/README.txt"

# Create a temporary writable DMG first
DMG_TEMP_RW="$RUNNER_TEMP/TabSaver-temp.dmg"
hdiutil create -size 100m -fs HFS+ -volname "TabSaver" "$DMG_TEMP_RW"

# Mount it
MOUNT_POINT="/Volumes/TabSaver"
hdiutil attach "$DMG_TEMP_RW"

# Copy plugins to the mounted DMG
if [ -d "$VST3_PATH" ]; then
    cp -r "$VST3_PATH" "$MOUNT_POINT/"
fi

if [ -d "$AU_PATH" ]; then
    cp -r "$AU_PATH" "$MOUNT_POINT/"
fi

# Copy README
cp README-MACOS.txt "$MOUNT_POINT/README.txt"

# Create alias/symlink to the user's plugin folders
# Note: We create these as AppleScript aliases that will resolve to the user's home
osascript <<EOF
tell application "Finder"
    make new alias file at POSIX file "$MOUNT_POINT" to POSIX file "/Library/Audio/Plug-Ins/VST3" with properties {name:"VST3"}
    make new alias file at POSIX file "$MOUNT_POINT" to POSIX file "/Library/Audio/Plug-Ins/Components" with properties {name:"Components"}
end tell
EOF

# Unmount the temporary DMG
hdiutil detach "$MOUNT_POINT"

# Convert to compressed read-only DMG
hdiutil convert "$DMG_TEMP_RW" -format UDZO -o "$DMG_NAME"

# Clean up temporary DMG
rm "$DMG_TEMP_RW"

echo "  ✓ DMG created: $DMG_NAME"
echo ""

# Sign DMG
echo "Signing DMG..."
codesign --force --sign "$IDENTITY" --timestamp "$DMG_NAME"
codesign -v "$DMG_NAME"
echo "  ✓ DMG signed successfully"
echo ""

# Notarize
echo "Submitting DMG for notarization..."
echo "This may take 5-15 minutes..."

xcrun notarytool submit "$DMG_NAME" \
    --apple-id "$APPLE_ID" \
    --password "$APPLE_ID_PASSWORD" \
    --team-id "$APPLE_TEAM_ID" \
    --wait

echo ""
echo "  ✓ Notarization complete"
echo ""

# Staple notarization ticket
echo "Stapling notarization ticket..."
xcrun stapler staple "$DMG_NAME"
echo "  ✓ Notarization ticket stapled"
echo ""

# Verify
echo "Verifying notarization..."
spctl -a -t open --context context:primary-signature -v "$DMG_NAME"
xcrun stapler validate "$DMG_NAME"
echo "  ✓ Verification successful"
echo ""

# Cleanup
rm certificate.p12
security delete-keychain "$KEYCHAIN_PATH"

echo "======================================"
echo "Code signing and notarization complete!"
echo "DMG ready for distribution: $DMG_NAME"
echo "======================================"
