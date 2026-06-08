#!/usr/bin/env bash
# Reproducible Vulkan AppImage build for DictaPulse.
# Captures the working recipe discovered on 2026-06-08 (Qt 6.11, KF6 6.26, Arch).
# Run from the repo root:  bash .appimage-tools/build-appimage.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
TOOLS="$ROOT/.appimage-tools"
CURATED="$TOOLS/qt-plugins-curated"
export VERSION="$(grep -oP 'VERSION \K[0-9]+\.[0-9]+\.[0-9]+' "$ROOT/CMakeLists.txt" | head -1)"
echo "==> building DictaPulse $VERSION AppImage"

# --- 0. tools (download once) -----------------------------------------------
[ -x "$TOOLS/linuxdeploy-x86_64.AppImage" ] || \
  wget -qO "$TOOLS/linuxdeploy-x86_64.AppImage" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
[ -x "$TOOLS/linuxdeploy-plugin-qt-x86_64.AppImage" ] || \
  wget -qO "$TOOLS/linuxdeploy-plugin-qt-x86_64.AppImage" https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
[ -x "$TOOLS/appimagetool-x86_64.AppImage" ] || \
  wget -qO "$TOOLS/appimagetool-x86_64.AppImage" https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x "$TOOLS"/*.AppImage

# --- 1. build (Vulkan) + stage into AppDir ----------------------------------
cmake -B build-appimage -S . -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DDICTAPULSE_ENABLE_VULKAN=ON
cmake --build build-appimage -j"$(nproc)"
rm -rf AppDir
DESTDIR="$ROOT/AppDir" cmake --install build-appimage
# AppImage convention: Exec must be the bare binary name, not an absolute path.
sed -i 's|^Exec=.*/dictapulse|Exec=dictapulse|' AppDir/usr/share/applications/dictapulse.desktop

# --- 2. curated Qt plugin tree ----------------------------------------------
# linuxdeploy-plugin-qt deploys EVERY plugin in QT_INSTALL_PLUGINS and dies if any
# has an unresolvable host dep (e.g. kimg_jxr->libjxrglue, libqsqlibase->libfbclient).
# Build a tree that drops ONLY plugins whose deps are missing on this host, then point
# the qt plugin at it via a qmake wrapper.
rm -rf "$CURATED"; mkdir -p "$CURATED"
while IFS= read -r so; do
  rel="${so#/usr/lib/qt6/plugins/}"; sub="$(dirname "$rel")"
  mkdir -p "$CURATED/$sub"
  if ldd "$so" 2>/dev/null | grep -q 'not found'; then
    echo "  drop $rel"
  else
    ln -s "$so" "$CURATED/$sub/$(basename "$so")"
  fi
done < <(find /usr/lib/qt6/plugins -name '*.so' -type f | sort)

cat > "$TOOLS/qmake-wrap" <<EOF
#!/usr/bin/env bash
if [ "\$1" = "-query" ] && [ -z "\${2:-}" ]; then
  /usr/bin/qmake6 -query | sed "s|^QT_INSTALL_PLUGINS:.*|QT_INSTALL_PLUGINS:$CURATED|"; exit \${PIPESTATUS[0]}
elif [ "\$1" = "-query" ] && [ "\$2" = "QT_INSTALL_PLUGINS" ]; then echo "$CURATED"; exit 0; fi
exec /usr/bin/qmake6 "\$@"
EOF
chmod +x "$TOOLS/qmake-wrap"

# --- 3. bundle Qt/KF6 with linuxdeploy --------------------------------------
export APPIMAGE_EXTRACT_AND_RUN=1
export NO_STRIP=1                       # bundled strip can't parse modern .relr.dyn
export QMAKE="$TOOLS/qmake-wrap"
export QML_SOURCES_PATHS="$ROOT/qml"
export EXTRA_PLATFORM_PLUGINS="libqwayland.so;libqoffscreen.so"   # else only xcb is bundled
"$TOOLS/linuxdeploy-x86_64.AppImage" --appdir AppDir --plugin qt \
  --desktop-file AppDir/usr/share/applications/dictapulse.desktop \
  --icon-file AppDir/usr/share/icons/hicolor/scalable/apps/dictapulse.svg

# --- 4. post-bundle fixes ----------------------------------------------------
# 4a. Wayland GPU rendering needs the EGL client integration plugin, which
#     linuxdeploy-plugin-qt does NOT auto-deploy. Without it QtQuick qFatal()s on
#     the first expose under a real Wayland compositor.
mkdir -p AppDir/usr/plugins/wayland-graphics-integration-client
cp -Lu /usr/lib/qt6/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so \
       AppDir/usr/plugins/wayland-graphics-integration-client/
# 4b. Do NOT bundle ffmpeg — the QtMultimedia ffmpeg backend SIGSEGVs in dlopen when
#     mixing a partial bundled ffmpeg with the host's. Host ffmpeg .62 is self-consistent.
( cd AppDir/usr/lib && rm -f libav*.so* libsw*.so* libpostproc*.so* )
# 4c. Vulkan: keep the bundled vendor-neutral loader (libvulkan.so.1) so the app always
#     launches; host ICDs (/usr/share/vulkan/icd.d) are used since we bundle no *.json.

# --- 5. package (appimagetool ships AppDir AS-IS — no dep re-resolution) -----
rm -f "DictaPulse-${VERSION}-x86_64.AppImage"
ARCH=x86_64 "$TOOLS/appimagetool-x86_64.AppImage" AppDir "DictaPulse-${VERSION}-x86_64.AppImage"
echo "==> built DictaPulse-${VERSION}-x86_64.AppImage"
