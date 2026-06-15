# Contributing to DictaPulse

Thanks for taking the time to help improve DictaPulse. The project is still in beta, so practical reports from real KDE Plasma systems are extremely useful. Bug reports, compatibility notes, packaging fixes, and focused pull requests all help move it forward.

## Good Places to Help

Useful places to start:

- Reproduce and narrow down KDE Plasma, Wayland, X11, `wtype`, or `ydotool` issues.
- Improve install, packaging, and release documentation.
- Fix small QML layout bugs in dark and light themes.
- Add missing runtime checks with clear user-facing error messages.
- Improve transcript cleanup rules, especially for multilingual punctuation.
- Test AppImage and AUR packages on real systems.

For larger features, please open a GitHub Discussion or issue first. It is much easier to agree on the shape of the change before a lot of code has been written.

## Privacy Rules

DictaPulse is local-first, privacy-first desktop software. Please keep that baseline intact:

- Do not add telemetry, analytics, tracking, or crash uploaders.
- Do not write microphone recordings to disk unless the user explicitly enables that behavior.
- Do not log transcript text, API keys, access tokens, or system keyring values.
- Store secrets through the existing keyring path, not config files.
- Treat remote cleanup providers as opt-in only.
- Keep host integration behavior clear to the user, especially text injection, clipboard use, KWin rules, and global shortcuts.

## License

By contributing to DictaPulse, you agree that your contribution is licensed under the same license as the project: GPL-3.0-or-later. Please only submit work you have the right to contribute.

Do not copy code, assets, models, fonts, or sounds from other projects unless their license is compatible and the attribution is included.

## Development Setup

DictaPulse targets KDE Plasma 6 with Qt 6.9+, KDE Frameworks 6, C++20, and QML.

On Arch or CachyOS:

```bash
sudo pacman -S --needed \
    qt6-base qt6-declarative qt6-multimedia qt6-svg qt6-wayland \
    extra-cmake-modules qtkeychain-qt6 \
    kglobalaccel kstatusnotifieritem knotifications kconfig \
    kcolorscheme kwindowsystem \
    cmake ninja gcc git wtype
```

Build and run:

```bash
git clone https://github.com/Silverhairfx/DictaPulse.git
cd DictaPulse
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
./build/src/dictapulse
```

Optional acceleration builds:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DDICTAPULSE_ENABLE_VULKAN=ON
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DDICTAPULSE_ENABLE_CUDA=ON
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DDICTAPULSE_ENABLE_HIP=ON
```

The convenience installer can also be used:

```bash
./scripts/install.sh
./scripts/install.sh --vulkan
```

## Code Style

- Follow the style already used in the files you touch.
- Prefer small, focused changes over broad refactors.
- Keep C++ at C++20.
- Use Qt/KDE APIs that match the existing architecture.
- Keep platform-specific behavior behind the platform adapter boundary.
- Keep QML components reusable and consistent with the existing clay design system.
- Avoid adding new dependencies unless they clearly reduce project risk or complexity.
- Add comments only where they clarify non-obvious behavior.

## Testing

There is not a full automated test suite yet, so manual verification matters.

For code changes, include what you tested in the pull request:

- Build result and CMake options used.
- KDE Plasma version.
- Qt version.
- Wayland or X11.
- Install method, if relevant.
- Dictation start/stop behavior.
- Text insertion mode tested: direct insertion, clipboard, or copy and paste.
- Model and backend mode tested.
- Any packaging command tested, such as `makepkg`, AppImage extraction, or install script execution.

Before opening a pull request, at minimum run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
```

For packaging changes, also verify the relevant package path under `packaging/aur/` and update `docs/PACKAGING.md` when the release process changes.

## Pull Requests

Pull requests should be focused and reviewable. If a change touches several unrelated areas, split it up.

Please include:

- What changed and why.
- Screenshots or screen recordings for UI changes.
- Manual test results.
- Any known limitations.
- Links to related issues or discussions.

I may ask for changes, ask you to split a large pull request, or close work that does not fit the roadmap or project constraints.

## Issues

Use the issue forms for bugs, feature requests, and packaging reports. Please include enough system detail for someone else to reproduce the behavior.

Security vulnerabilities should not be reported in public issues. See [.github/SECURITY.md](.github/SECURITY.md).
