# DictaPulse roadmap

DictaPulse ships on Linux today. macOS and Windows builds are in private testing
and will be published per platform once they pass QA. New features land on Linux
first, then roll to macOS and Windows as those platforms reach parity.

## Platform status

| Platform | Status |
|---|---|
| Linux (KDE Plasma, Wayland) | Released, actively maintained |
| macOS | In private testing; public release after QA |
| Windows | Later; outstanding bugs to fix first, then feature parity |

## Released (v0.1.x)

KDE Plasma voice dictation: whisper.cpp + Vulkan, the listening overlay, full
settings, rules and local/remote LLM cleanup, personal dictionary, and the
profile system.

## In progress (v0.2.0, Linux)

- Cleanup model picker: choose the local LLM (LM Studio / Ollama) from a dropdown
  of installed models, with a guided "download a model" flow and automatic model
  loading on use.
- GPU build hardening: Vulkan auto-enabled on Linux, automatic CPU fallback when
  no device is present.
- Overlay start-recording cue fixed (an idle audio sink no longer swallows it).
- Text vertical alignment corrected across all input fields.
- Code comment cleanup.

## Planned (v0.3.0, Linux): auto-learning vocabulary

Learn the user's preferred spellings from the corrections they make inside
DictaPulse, without reading other applications (not possible on Wayland anyway).

- Transcript history: each dictation is saved locally, on device, never uploaded.
- Review and correct: edit a transcript in the app; the diff against the original
  produces candidate "heard as -> written as" entries.
- Learn: confirmed corrections become personal-dictionary entries that bias
  Whisper and apply to future transcripts.
- Retention and cleanup (keeps storage small):
  - Per-entry delete and a "clear all" button.
  - Automatic deletion after a configurable age (off / 7 / 30 / 90 days) plus a
    max-entry cap, so history never grows unbounded.
  - Learned dictionary words persist independently and stay editable in
    Profile > Personalize.

### Streaming / chunked transcription

Today a recording is transcribed in a single pass after you stop, so a long
dictation means a long wait before the text appears. Transcribe in chunks while
you speak so text lands incrementally and long recordings paste with little to no
extra delay.

## Later

- macOS: bring the new features to parity, then publish.
- Windows: fix outstanding issues, reach parity, then publish.
- Live partial transcripts, additional packaging channels.
