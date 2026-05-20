# DictaPulse

Local AI voice dictation for KDE Plasma 6.6+, built with C++ and Qt 6 Quick/QML.
Cross-platform core with desktop adapters for Linux first, then Windows, macOS,
Android (IME), and iOS (keyboard extension).

> Status: scaffold. The app builds and launches a placeholder window. Features
> land incrementally.

## Build

Requires Qt 6.6+, CMake 3.21+, and a C++20 compiler. Optional KDE Frameworks 6
integration is auto-detected when `extra-cmake-modules` and the relevant KF6
components are present.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/dictapulse
```

### Arch / CachyOS dependencies

```sh
sudo pacman -S --needed \
    qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-wayland \
    extra-cmake-modules \
    kglobalaccel kstatusnotifieritem knotifications kconfig \
    cmake ninja gcc
```

## Architecture

```
DictaPulse/
├── src/                C++ sources
│   └── main.cpp        QGuiApplication + QML engine entrypoint
├── qml/                QML UI (module URI: DictaPulse)
│   └── Main.qml
└── CMakeLists.txt
```

The codebase will grow into the layered architecture from the product spec:

- **UI** (QML): overlay, settings, model manager, tray, onboarding
- **Core engine** (C++): audio capture, VAD, STT runtime, model management,
  language support, post-processing
- **Platform adapters** (C++): KDE/Wayland, X11, Windows, macOS, Android IME,
  iOS keyboard extension
- **Packaging**: AppImage / Flatpak / AUR, MSIX, DMG, mobile stores

## License

TBD.
