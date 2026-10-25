#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
if [[ "$(uname -s)" == Darwin ]]; then
  bundle="build/Rue du Soir.app/Contents"
  mkdir -p "$bundle/MacOS"
  ln -sfn ../../../dusk-street "$bundle/MacOS/dusk-street"
  cat > "$bundle/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleExecutable</key><string>dusk-street</string>
<key>CFBundleIdentifier</key><string>fr.polytech.rue-du-soir</string>
<key>CFBundleName</key><string>Rue du Soir</string>
<key>CFBundlePackageType</key><string>APPL</string>
<key>CFBundleVersion</key><string>1</string>
<key>NSHighResolutionCapable</key><false/>
</dict></plist>
PLIST
fi
if [[ "${1:-}" == "--run" ]]; then
  shift
  exec ./build/dusk-street "$@"
fi
