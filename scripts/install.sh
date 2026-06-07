#!/usr/bin/env bash
# DictaPulse installer for Arch / CachyOS (and any distro with Qt 6.9+, KF6, and a C++20 compiler).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"
PREFIX="${PREFIX:-$HOME/.local}"

GREEN='\033[1;32m'; YELLOW='\033[1;33m'; RED='\033[1;31m'; RESET='\033[0m'
say() { printf "${GREEN}==>${RESET} %s\n" "$*"; }
warn() { printf "${YELLOW}!! ${RESET} %s\n" "$*"; }
die()  { printf "${RED}xx ${RESET} %s\n" "$*" >&2; exit 1; }

# --- Dependency check --------------------------------------------------------
need() { command -v "$1" >/dev/null 2>&1 || die "missing dependency: $1"; }
need cmake
need g++
need git

if command -v pacman >/dev/null 2>&1; then
    missing=()
    for pkg in qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-wayland \
               extra-cmake-modules kglobalaccel kstatusnotifieritem \
               knotifications kconfig kcolorscheme kwindowsystem qtkeychain-qt6 wtype; do
        pacman -Q "$pkg" >/dev/null 2>&1 || missing+=("$pkg")
    done
    if [ ${#missing[@]} -gt 0 ]; then
        warn "Missing packages: ${missing[*]}"
        say  "Run: sudo pacman -S --needed ${missing[*]}"
        read -r -p "Install now? [y/N] " ans
        [[ "${ans,,}" == "y" ]] && sudo pacman -S --needed "${missing[@]}"
    fi
fi

# --- Build options -----------------------------------------------------------
CMAKE_FLAGS=( -S "${REPO_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${PREFIX}" )

for arg in "$@"; do
    case "$arg" in
        --vulkan)  CMAKE_FLAGS+=( -DDICTAPULSE_ENABLE_VULKAN=ON ) ;;
        --cuda)    CMAKE_FLAGS+=( -DDICTAPULSE_ENABLE_CUDA=ON ) ;;
        --hip|--rocm) CMAKE_FLAGS+=( -DDICTAPULSE_ENABLE_HIP=ON ) ;;
        --debug)   CMAKE_FLAGS=( "${CMAKE_FLAGS[@]/-DCMAKE_BUILD_TYPE=Release/-DCMAKE_BUILD_TYPE=Debug}" ) ;;
        --prefix=*) PREFIX="${arg#*=}" ;;
        *) warn "unknown flag: $arg" ;;
    esac
done

say "Configuring → ${BUILD_DIR}"
cmake "${CMAKE_FLAGS[@]}"

say "Building (this fetches and compiles whisper.cpp on first run — grab a coffee)"
cmake --build "${BUILD_DIR}" -j "$(nproc)"

say "Installing into ${PREFIX}"
cmake --install "${BUILD_DIR}"

# Refresh launcher caches so the new .desktop and icon are visible immediately.
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${PREFIX}/share/applications" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor" 2>/dev/null || true
fi
# KDE: rebuild the system configuration cache (covers app menu + icon lookup).
if command -v kbuildsycoca6 >/dev/null 2>&1; then
    kbuildsycoca6 --noincremental 2>/dev/null || true
elif command -v kbuildsycoca5 >/dev/null 2>&1; then
    kbuildsycoca5 --noincremental 2>/dev/null || true
fi

cat <<EOF

${GREEN}✓ DictaPulse installed.${RESET}
  Binary:   ${PREFIX}/bin/dictapulse
  Desktop:  ${PREFIX}/share/applications/dictapulse.desktop

Make sure ${PREFIX}/bin is on your PATH:
  echo 'set -gx PATH \$HOME/.local/bin \$PATH' >> ~/.config/fish/config.fish    # fish
  echo 'export PATH="\$HOME/.local/bin:\$PATH"' >> ~/.bashrc                    # bash/zsh

First run:
  1. Launch ${YELLOW}dictapulse${RESET} (or find it in your KDE launcher).
  2. Open Settings → Models, download "Base (multilingual)" (~150 MB).
  3. Set your shortcut on the Shortcuts page, hit "Apply shortcuts".
  4. Press the shortcut from any text field and start speaking.

EOF
