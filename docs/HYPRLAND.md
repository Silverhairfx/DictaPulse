# DictaPulse on Hyprland

DictaPulse detects the desktop at runtime. Under KDE Plasma it keeps using KGlobalAccel,
KStatusNotifierItem and KWin. Under Hyprland (and other wlroots-style Wayland
compositors) it uses the pieces below instead. Nothing is forked: both paths live in the
same binary.

## What you need

- `wtype` (virtual keyboard, used to type Unicode text, including Arabic)
- `wl-clipboard` (`wl-copy`, used when text has to go through the clipboard)
- `layer-shell-qt` (the listening overlay is a layer-shell surface)
- A StatusNotifierItem tray host (DankMaterialShell, waybar, KDE, ...) for the tray icon

## Key binds

Hyprland has no global-shortcut service of its own, so keys are bound by the compositor to
small commands that talk to the running instance over a per-user local socket
(`$XDG_RUNTIME_DIR/dictapulse/control.sock`):

| Command | Effect |
|---|---|
| `dictapulse --toggle` | start dictation, or stop and insert |
| `dictapulse --ptt-press` | start dictation (hold-to-talk, key down) |
| `dictapulse --ptt-release` | stop and insert (hold-to-talk, key up) |
| `dictapulse --cancel` | discard the current dictation |
| `dictapulse --status` | print the current state |
| `dictapulse --show` | open the settings window |
| `dictapulse --quit` | quit the running instance |

Example for a Lua configuration (tested on Hyprland 0.57):

```lua
local dp = os.getenv("HOME") .. "/.local/bin/dictapulse"
hl.bind("SUPER + CTRL + SHIFT + A", hl.dsp.exec_cmd(dp .. " --toggle"))
hl.bind("SUPER + ALT + R", hl.dsp.exec_cmd(dp .. " --ptt-press"))
hl.bind("SUPER + ALT + R", hl.dsp.exec_cmd(dp .. " --ptt-release"),
        { release = true, ignore_mods = true, non_consuming = true })
hl.bind("SUPER + CTRL + SHIFT + X", hl.dsp.exec_cmd(dp .. " --cancel"))
```

The same thing in hyprlang:

```ini
bind   = SUPER CTRL SHIFT, A, exec, dictapulse --toggle
bind   = SUPER ALT, R, exec, dictapulse --ptt-press
bindrni = SUPER ALT, R, exec, dictapulse --ptt-release
bind   = SUPER CTRL SHIFT, X, exec, dictapulse --cancel
```

The release bind ignores modifiers so that letting go of the modifier keys before the
letter key still ends the recording. A tap shorter than a quarter of a second is treated
as accidental and discarded.

## Where the text goes

- When dictation starts, the focused window is recorded from Hyprland's IPC
  (`j/activewindow`: address, class, title).
- When the text is ready, the focused window is checked again. **If focus moved, nothing
  is typed.** The text is put on the clipboard and a notification says
  "Focus changed, text copied".
- Text is typed through the virtual keyboard (`wtype`), which carries its own keymap, so
  Arabic and English come out correctly whatever keyboard layout is active.
- Line breaks are never typed as Enter. In a terminal they become spaces. In other
  applications multi-line text is pasted. DictaPulse presses Enter only when
  `output/pressEnterAfterInsert=true` is set in the settings file, and then only once,
  after a successful insertion into the original window.
- Paste uses `ctrl+shift+v` in terminal emulators (recognised by window class or, for a custom
  class, by process name: kitty, foot, alacritty, wezterm,
  konsole, ghostty, ...) and `ctrl+v` everywhere else.

## Microphone

The system default input is never changed. DictaPulse picks the first input whose id or
description contains an entry of `audio/preferredMicrophones` (comma separated,
case-insensitive, default `boya,by-cm3`), otherwise the system default. Set
`audio/inputDevice` to an exact device id to force one.

## Models in place

Models already on disk can be used without copying them: add the directory to the
`models/extraDirs` setting (or `DICTAPULSE_MODEL_DIRS`, colon separated). A file named
`whisper-large-v3-q8_0.gguf` there appears as "Large v3 Q8 (multilingual, local file)".

## Testing without a microphone

`DICTAPULSE_TEST_AUDIO=/path/to/file.wav` replaces the microphone with a recorded 16-bit
PCM WAV file, and `DICTAPULSE_CONTROL_SOCKET=/path/to/sock` runs a second, isolated
instance. `ctest` covers the focus guard, paste chord selection, the Arabic text path,
the no-Enter default, the IPC parser and the control socket.
