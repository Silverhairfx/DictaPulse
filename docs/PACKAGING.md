# Packaging & Releasing DictaPulse

This is the step-by-step playbook for shipping DictaPulse to users. Every command
here is meant to be run **by a human** — nothing in this repo runs it automatically.

> **Context that drives every decision below.** DictaPulse is a *KDE host-integration*
> app, not a self-contained one. It:
> - registers global shortcuts via **KGlobalAccel** (`src/platform/linux/LinuxAdapter.cpp`),
> - writes a **KWin window rule** to `~/.config/kwinrulesrc` and calls
>   `org.kde.KWin → reconfigure` for overlay positioning (`src/main.cpp`),
> - **loads a KWin script over D-Bus** to watch the focused window
>   (`src/platform/linux/ActiveWindowWatcher.cpp`),
> - injects text with **`wtype`** (Wayland) / **`ydotool`** (uinput) and uses Klipper +
>   a layout switch (`src/platform/linux/TextInjector.*`).
>
> All of that only works when the app runs **natively on the host**, sharing the user's
> D-Bus session and Plasma services. This is why **AUR and AppImage work but Flathub
> does not** (see §4). Keep this in mind before investing in any channel.

**Recommended strategy**

| Channel | Verdict | Role |
|---|---|---|
| GitHub tagged source release | ✅ do it | Canonical release + changelog + tarball (§1) |
| AUR (`PKGBUILD`) | ✅ best fit | Primary install path for the target audience (§2) |
| AppImage (Vulkan) | ✅ works | "Download & run" binary, attached to the release (§3) |
| Flatpak / Flathub | ⛔ deferred | Needs an injection rewrite around XDG portals (§4) |

Current version is `0.1.0` (set in `CMakeLists.txt:project(... VERSION 0.1.0)`).
Bump that line first whenever you cut a new version.

---

## 1. GitHub source release

A source release is the canonical anchor: it creates the `vX.Y.Z` git tag and the
auto-generated source tarball that the AUR `PKGBUILD` (§2) downloads, and it's where you
attach the AppImage (§3).

### 1.1 Pre-flight

```bash
cd ~/github/DictaPulse
git status            # must be clean
git log -1 --oneline  # confirm HEAD is what you want to ship

# Make sure the version in CMakeLists.txt matches the tag you're about to make.
grep -n 'VERSION' CMakeLists.txt
```

### 1.2 Tag and push the tag

```bash
VERSION=0.1.0
git tag -a "v${VERSION}" -m "DictaPulse ${VERSION}"
git push origin "v${VERSION}"
```

> `git push origin vX.Y.Z` pushes **only the tag**, not your branch. Tags are awkward to
> retract once others fetch them — only do this when HEAD is final.

### 1.3 Create the release

Write notes to a file so they're version-controllable and reusable:

```bash
gh release create "v${VERSION}" \
  --repo Silverhairfx/DictaPulse \
  --title "DictaPulse ${VERSION}" \
  --notes-file docs/release-notes/v${VERSION}.md \
  --verify-tag
```

Or use `--draft` to stage it on github.com and publish from the web UI after review:

```bash
gh release create "v${VERSION}" --draft --notes-file docs/release-notes/v${VERSION}.md
```

GitHub automatically attaches `Source code (tar.gz)` and `(zip)` — that's all the AUR
`PKGBUILD` needs.

### 1.4 Attach the AppImage (after §3)

```bash
gh release upload "v${VERSION}" DictaPulse-${VERSION}-x86_64.AppImage
```

### Release-notes template (`docs/release-notes/vX.Y.Z.md`)

```markdown
## DictaPulse vX.Y.Z

### Highlights
- ...

### Install
- **Arch / CachyOS:** `paru -S dictapulse` (AUR), or build from source (see README).
- **Any distro:** download `DictaPulse-X.Y.Z-x86_64.AppImage` below, `chmod +x`, run.
  Requires KDE Plasma 6 (Wayland) + a running `ydotoold`. See README for the full
  runtime story.

### Changes
- ...

**Full changelog:** https://github.com/Silverhairfx/DictaPulse/compare/vPREV...vX.Y.Z
```

---

## 2. AUR — the primary channel

AUR builds from source **on the user's machine** against their native Qt 6.9 / KF6, then
installs to `/usr`. No sandbox, no bundling — injection works exactly as in development.
This is the best fit for the Arch/CachyOS + Plasma 6 audience.

You'll publish two related package bases over time:
- `dictapulse` — builds a tagged release (stable, points at the §1 tarball).
- `dictapulse-git` — builds from `main` (optional, for early adopters).

### 2.1 `dictapulse` PKGBUILD (release build)

> **Verified 2026-06-08** against the **v0.1.1** tarball in a fully offline `build()`:
> `makepkg` succeeds and the package contains `usr/share/applications/dictapulse.desktop`
> (the DESTDIR `.desktop` fix is what makes `package()` work — pre-fix it would write to the
> real `/usr` and fail). The canonical, tested PKGBUILD + `.SRCINFO` live in the repo at
> **`packaging/aur/dictapulse/`** — the block below mirrors them.

> **Clean-chroot safe.** whisper.cpp is normally pulled by CMake FetchContent during
> `build()`, which needs network and **breaks `--nonetwork` clean-chroot builds**. This
> PKGBUILD instead **vendors whisper.cpp `v1.7.6` as a real `source=()`** and points
> `-DFETCHCONTENT_SOURCE_DIR_WHISPER_CPP` at the unpacked tree with
> `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`, so the compile never touches the network. On a
> future version bump, keep `_whisperver` in sync with the `GIT_TAG` in `CMakeLists.txt`.

```bash
# Maintainer: Tymour Kadry <tymitaly@gmail.com>
pkgname=dictapulse
pkgver=0.1.1
pkgrel=2
_whisperver=1.7.6
pkgdesc="Local AI voice dictation for KDE Plasma (Wayland)"
arch=('x86_64')
url="https://github.com/Silverhairfx/DictaPulse"
license=('GPL-3.0-or-later')
depends=(
  qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-wayland
  kglobalaccel kstatusnotifieritem knotifications kconfig
  kcolorscheme kwindowsystem qtkeychain-qt6
  hicolor-icon-theme
)
# Runtime injection helpers — at least one of wtype/ydotool is required at runtime.
optdepends=(
  'ydotool: text injection via uinput (needs the ydotoold daemon running)'
  'wtype: text injection on wlroots/Wayland'
)
makedepends=(cmake extra-cmake-modules git
             vulkan-headers vulkan-icd-loader shaderc)  # shaderc = glslc, for Vulkan shaders
source=(
  "$pkgname-$pkgver.tar.gz::$url/archive/refs/tags/v$pkgver.tar.gz"
  "whisper.cpp-$_whisperver.tar.gz::https://github.com/ggml-org/whisper.cpp/archive/refs/tags/v$_whisperver.tar.gz"
)
sha256sums=('c2745aaf50b330a231c53a2d03da44f5aea44da96818c6e8403be296e68658a3'
            '166140e9a6d8a36f787a2bd77f8f44dd64874f12dd8359ff7c1f4f9acb86202e')

build() {
  cd "DictaPulse-$pkgver"
  cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DDICTAPULSE_ENABLE_VULKAN=ON \
    -DFETCHCONTENT_SOURCE_DIR_WHISPER_CPP="$srcdir/whisper.cpp-$_whisperver" \
    -DFETCHCONTENT_FULLY_DISCONNECTED=ON
  cmake --build build -j"$(nproc)"
}

package() {
  cd "DictaPulse-$pkgver"
  DESTDIR="$pkgdir" cmake --install build
}
```

> **namcap note:** the source package is clean apart from namcap's known Qt/KDE blind spots —
> the embedded `DictaPulse` QML module, base libs "implicitly satisfied", and "may not be
> needed" warnings for `qt6-wayland` / `qt6-svg` / `kconfig` / `kwindowsystem`. Those are
> **runtime plugin deps** (e.g. the Wayland QPA plugin) that namcap can't see being linked —
> keep them. The `dictapulse-bin` package additionally trips `ELF files outside /opt`,
> `unstripped`, and "unused shared library" — all expected for a self-contained AppImage
> bundle and accepted for `-bin` packages.

### 2.2 Build & test locally before publishing

```bash
mkdir -p /tmp/aur-dictapulse && cd /tmp/aur-dictapulse
# paste the PKGBUILD here
makepkg -g >> PKGBUILD          # generate real sha256sums, then move the line up/replace SKIP
makepkg -si                     # build + install, confirm the app launches & injects
namcap PKGBUILD                 # lint the PKGBUILD
namcap dictapulse-*.pkg.tar.zst # lint the built package (catches missing deps)
```

`namcap` flags under/over-declared dependencies — run it until clean. In particular it
will tell you if a `depends` entry is actually pulled in transitively (trim those) or if
you link something not declared (add it).

### 2.3 Publish to the AUR

```bash
# One-time: add your SSH key to https://aur.archlinux.org → My Account
git clone ssh://aur@aur.archlinux.org/dictapulse.git
cd dictapulse
cp /tmp/aur-dictapulse/PKGBUILD .
makepkg --printsrcinfo > .SRCINFO   # REQUIRED — the AUR rejects pushes without it
git add PKGBUILD .SRCINFO
git commit -m "dictapulse 0.1.0"
git push
```

> The AUR is a separate git host from GitHub. `.SRCINFO` must be regenerated and committed
> on **every** version bump or the web UI shows stale metadata.

### 2.4 Updating later

```bash
# bump pkgver (and reset pkgrel=1), refresh sums, regenerate .SRCINFO, commit, push
makepkg -g                          # new sha256sums
makepkg --printsrcinfo > .SRCINFO
git commit -am "dictapulse X.Y.Z" && git push
```

---

## 3. AppImage (Vulkan) — the download-and-run binary

An AppImage runs **unsandboxed** on the host, so KGlobalAccel / KWin / ydotool / Klipper
all behave exactly as a native install — that's why it works for this app where Flatpak
won't. whisper's **Vulkan** backend loads the *host* Vulkan driver at runtime, so a single
image covers AMD/Intel/NVIDIA without bundling any vendor toolkit.

> **Just run the script.** The full working recipe lives in
> `scripts/build-appimage.sh` (built & verified 2026-06-08 on Qt 6.11 / KF6 6.26).
> From the repo root: `bash scripts/build-appimage.sh` → produces
> `DictaPulse-<version>-x86_64.AppImage` (~114 MB). The rest of this section explains the
> **six non-obvious gotchas** that script encodes — read it before changing the script,
> because every one of these was a crash or a build failure that had to be debugged.

Trade-offs: the image bundles Qt 6.11 + KF6 (~114 MB), and because KF6 is recent you must
build on a current base (up-to-date Arch is ideal). It deliberately relies on a few host
libraries (ffmpeg, Vulkan ICDs) — see below.

### 3.1 Build (Vulkan) + stage into AppDir

```bash
cmake -B build-appimage -S . -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DDICTAPULSE_ENABLE_VULKAN=ON
cmake --build build-appimage -j"$(nproc)"
rm -rf AppDir && DESTDIR="$PWD/AppDir" cmake --install build-appimage
sed -i 's|^Exec=.*/dictapulse|Exec=dictapulse|' AppDir/usr/share/applications/dictapulse.desktop
```

whisper.cpp is built **static**, so it's already inside the `dictapulse` binary.

> **Gotcha 1 — DESTDIR.** The `.desktop` file is written by `file(WRITE)` inside
> `install(CODE …)` in `src/CMakeLists.txt`, which originally ignored `$ENV{DESTDIR}` and
> tried to write to the real `/usr` (permission denied). Fixed by prepending `$ENV{DESTDIR}`
> to the dest dir — **this also fixes the AUR `package()` step**, which installs the same way.
>
> **Gotcha 2 — Exec path.** The native install uses an absolute `Exec=/usr/bin/dictapulse`
> (needed for the KDE launcher), but AppImage requires the **bare binary name** or
> linuxdeploy fails with "could not find suitable executable for Exec entry". Hence the `sed`.

### 3.2 Curated Qt plugin tree (Gotcha 3)

`linuxdeploy-plugin-qt` deploys **every** plugin in `QT_INSTALL_PLUGINS` and aborts the whole
build if any one has a host dependency it can't resolve. On a full KDE box that means
`kimg_jxr.so → libjxrglue`, `libqsqlibase.so → libfbclient`, sonnet spellcheckers, VPN
backends, etc. — ~30 plugins DictaPulse never uses. Solution: point the qt plugin at a
curated tree that drops **only** plugins with unresolvable deps (keeping `libqsvg`,
`libqsqlite`, `libqwayland`, the ffmpeg multimedia plugin, …) via a `qmake` wrapper that
overrides `QT_INSTALL_PLUGINS`. See the script's section 2.

### 3.3 Bundle with linuxdeploy (Gotchas 4 & 5)

```bash
export APPIMAGE_EXTRACT_AND_RUN=1   # FUSE is unreliable in CI/containers
export NO_STRIP=1                   # Gotcha 4
export QMAKE="$PWD/.appimage-tools/qmake-wrap"
export QML_SOURCES_PATHS="$PWD/qml"
export EXTRA_PLATFORM_PLUGINS="libqwayland.so;libqoffscreen.so"   # Gotcha 5
./.appimage-tools/linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt \
  --desktop-file AppDir/usr/share/applications/dictapulse.desktop \
  --icon-file AppDir/usr/share/icons/hicolor/scalable/apps/dictapulse.svg
```

> **Gotcha 4 — NO_STRIP.** linuxdeploy's bundled `strip` is too old to parse modern
> `.relr.dyn` (RELR) relocation sections and errors out on every lib. `NO_STRIP=1` skips it.
>
> **Gotcha 5 — platform plugins.** By default linuxdeploy-plugin-qt bundles **only
> `libqxcb.so`**. DictaPulse is Wayland-native, so without `EXTRA_PLATFORM_PLUGINS` forcing
> `libqwayland.so` the app has no Wayland QPA plugin at all. (`libqoffscreen.so` is added
> for headless smoke-testing.) Including the wayland platform makes the plugin also pull
> `wayland-shell-integration` (incl. `libxdg-shell.so`, `liblayer-shell.so`) and
> `wayland-decoration-client` automatically.

KF6 libs (`libKF6GlobalAccel`, `libKF6StatusNotifierItem`, …) are linked by the binary, so
linuxdeploy follows and bundles them automatically.

### 3.4 Post-bundle fixes (Gotcha 6 + Vulkan + ffmpeg)

```bash
# 6a. Wayland GPU rendering needs the EGL client integration plugin, which
#     linuxdeploy-plugin-qt does NOT auto-deploy. Without it QtQuick qFatal()s on the
#     first expose under a real compositor (offscreen/software rendering hides this).
mkdir -p AppDir/usr/plugins/wayland-graphics-integration-client
cp -Lu /usr/lib/qt6/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so \
       AppDir/usr/plugins/wayland-graphics-integration-client/

# 6b. Do NOT bundle ffmpeg. The QtMultimedia ffmpeg backend SIGSEGVs inside dlopen() when a
#     partially-bundled ffmpeg mixes with the host's. (QSoundEffect/QMediaDevices probe audio
#     devices on startup → loads libffmpegmediaplugin.so → crash.) Host ffmpeg .62 is
#     self-consistent and present on essentially every desktop.
( cd AppDir/usr/lib && rm -f libav*.so* libsw*.so* libpostproc*.so* )
```

> **Vulkan loader:** *keep* the bundled vendor-neutral `libvulkan.so.1` (do **not** strip it —
> the original guidance here was wrong). Bundling the loader guarantees the app launches and
> falls back to CPU gracefully; the host **ICDs** (`/usr/share/vulkan/icd.d/*.json` + driver)
> are still used because we bundle **no** `*_icd.json`. Stripping the loader risks an app that
> won't start on hosts lacking `libvulkan.so.1`.

### 3.5 Package with appimagetool (not linuxdeploy --output)

```bash
ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 \
  ./.appimage-tools/appimagetool-x86_64.AppImage AppDir DictaPulse-0.1.0-x86_64.AppImage
```

Use **appimagetool directly** for the final package — it ships `AppDir` *as-is*. Re-running
`linuxdeploy --output appimage` would re-resolve dependencies and **re-add the ffmpeg libs**
you just removed in 6b, reintroducing the crash.

### 3.6 Verify (this app's smoke test)

The honest test is launching against a real session. Because it's a GUI app, run it
time-boxed and check it survives past scene-graph init (offscreen catches lib/QML problems;
**real Wayland** catches the EGL/render problems offscreen hides — test both):

```bash
./DictaPulse-0.1.0-x86_64.AppImage --appimage-extract >/dev/null   # → squashfs-root/
for plat in offscreen wayland; do
  ( QT_QPA_PLATFORM=$plat ./squashfs-root/usr/bin/dictapulse >/tmp/v-$plat.log 2>&1 ) &
  pid=$!; sleep 8
  if kill -0 $pid 2>/dev/null; then echo "[$plat] RUNNING ✓"; kill -9 $pid; else echo "[$plat] crashed $?"; fi
done
```

A healthy run reaches `[DictaPulse] overlay kwin-rule: pos=… size=…` and keeps running.
For a full functional check, run it normally and confirm: shortcut registers, overlay
positions, dictation injects text. (Best done on a *clean* user/machine to prove bundling
is complete.)

### 3.7 Runtime prerequisites the AppImage can NOT bundle

State these in the release notes — they're host-level and outside the image:
- **KDE Plasma 6 on Wayland** (the overlay KWin rule + KWin script assume KWin).
- **`ydotoold` running** with `/dev/uinput` access (for the ydotool injection path), or a
  working `wtype` on the compositor.
- **ffmpeg (libav* .62)** — intentionally not bundled (Gotcha 6b); present on virtually every
  desktop. If absent, audio device probing / sound cues fail.
- **Vulkan ICD + driver** (`vulkan-icd-loader` + mesa/NVIDIA) for GPU acceleration. The
  loader is bundled but ICDs are not; without a driver, whisper falls back to CPU.
- A whisper **model** is downloaded on first run into the user's data dir (kept out of the
  image deliberately).

---

## 4. Flatpak / Flathub — why it's deferred

Flatpak sandboxes the app and routes privileged actions through **XDG desktop portals**.
DictaPulse's entire integration layer bypasses portals and talks to Plasma directly, so the
sandbox breaks it. Concretely:

| DictaPulse does | Sandbox reality | Blessed Flatpak path |
|---|---|---|
| `ydotool`/uinput text injection | `/dev/uinput` access is rejected by Flathub review | **RemoteDesktop portal** (rewrite) |
| `KGlobalAccel` global shortcut | Not reachable without a broad `--talk-name` hole | **GlobalShortcuts portal** (rewrite) |
| Writes `~/.config/kwinrulesrc` + `KWin reconfigure` | Host config is outside the sandbox | No portal — not doable cleanly |
| Loads a **KWin script** over D-Bus | Can't talk to `org.kde.KWin` without holes | No portal — not doable cleanly |
| Klipper D-Bus + keyboard-layout switch | Needs `--talk-name org.kde.klipper` + more | Discouraged by reviewers |

You *could* force it through with `--device=all` and several `--talk-name`/`--system-talk`
holes, but that guts the sandbox and **Flathub will reject** such a manifest. The honest
position: **Flathub is blocked until injection + shortcuts + overlay are re-architected
around XDG portals** (RemoteDesktop + GlobalShortcuts), at which point the KWin-script
window watcher also needs a portal-friendly replacement.

If/when you do that rewrite, you'll need the two reverse-DNS-named files below (also handy
for AppImage update metadata). Flathub's recommended app ID for a GitHub-hosted project is
`io.github.<user>.<Repo>` → **`io.github.Silverhairfx.DictaPulse`**. The existing
`resources/desktop/dictapulse.desktop.in` and icon would be renamed to match.

### 4.1 AppStream metainfo (also recommended for AppImage now)

Save as `resources/metainfo/io.github.Silverhairfx.DictaPulse.metainfo.xml` and install it
to `/usr/share/metainfo/`. Required for Flathub; improves AppImage/store listings today.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>io.github.Silverhairfx.DictaPulse</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0-or-later</project_license>
  <name>DictaPulse</name>
  <summary>Local AI voice dictation for KDE Plasma</summary>
  <description>
    <p>
      DictaPulse turns speech into polished text anywhere on KDE Plasma 6,
      running whisper.cpp locally with optional Vulkan GPU acceleration.
    </p>
  </description>
  <launchable type="desktop-id">io.github.Silverhairfx.DictaPulse.desktop</launchable>
  <url type="homepage">https://github.com/Silverhairfx/DictaPulse</url>
  <url type="bugtracker">https://github.com/Silverhairfx/DictaPulse/issues</url>
  <developer id="io.github.Silverhairfx">
    <name>Tymour Kadry</name>
  </developer>
  <content_rating type="oars-1.1"/>
  <releases>
    <release version="0.1.0" date="2026-06-08"/>
  </releases>
</component>
```

Validate it before relying on it:

```bash
appstreamcli validate resources/metainfo/io.github.Silverhairfx.DictaPulse.metainfo.xml
```

### 4.2 Manifest sketch (NOT submission-ready — documents the holes)

This is recorded so the blockers are explicit. It is intentionally not a clean manifest.

```yaml
# io.github.Silverhairfx.DictaPulse.yaml  — KDE runtime base
app-id: io.github.Silverhairfx.DictaPulse
runtime: org.kde.Platform
runtime-version: '6.9'
sdk: org.kde.Sdk
command: dictapulse
finish-args:
  - --share=ipc
  - --socket=wayland
  - --socket=pulseaudio          # mic capture
  - --device=dri                 # Vulkan
  # --- everything below is why Flathub will reject this ---
  - --device=all                 # /dev/uinput for ydotool  ← reviewers reject
  - --talk-name=org.kde.kglobalaccel
  - --talk-name=org.kde.KWin
  - --talk-name=org.kde.klipper
modules:
  # whisper.cpp must be a pinned git/archive module (no network at build time
  # under flatpak-builder) instead of CMake FetchContent.
  - name: dictapulse
    buildsystem: cmake-ninja
    config-opts:
      - -DDICTAPULSE_ENABLE_VULKAN=ON
    sources:
      - type: dir
        path: .
```

When you revisit this seriously, the work is: replace ydotool→RemoteDesktop portal,
KGlobalAccel→GlobalShortcuts portal, drop the KWin rule/script in favor of a portal- or
protocol-based overlay + window query, then the manifest's privileged `finish-args` all
disappear and submission to https://github.com/flathub/flathub becomes viable.

---

## Quick reference — cut a release end to end

```bash
# 0. bump VERSION in CMakeLists.txt, commit
VERSION=0.1.0

# 1. tag + GitHub source release
git tag -a "v${VERSION}" -m "DictaPulse ${VERSION}"
git push origin "v${VERSION}"
gh release create "v${VERSION}" --title "DictaPulse ${VERSION}" --notes-file docs/release-notes/v${VERSION}.md

# 2. AppImage, then attach it
#    (run the §3 build, producing DictaPulse-${VERSION}-x86_64.AppImage)
gh release upload "v${VERSION}" "DictaPulse-${VERSION}-x86_64.AppImage"

# 3. AUR: bump pkgver, refresh sums + .SRCINFO, push
#    (in the aur.archlinux.org/dictapulse.git clone)
makepkg -g && makepkg --printsrcinfo > .SRCINFO
git commit -am "dictapulse ${VERSION}" && git push
```
