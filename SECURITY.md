# Security Policy

DictaPulse is local-first dictation software, so security and privacy matter a lot. The app handles microphone input, text insertion, clipboard operations, local models, optional remote cleanup providers, and stored API keys. If you find a vulnerability, please report it privately first.

## Supported Versions

DictaPulse is currently in public beta.

Security fixes are handled for:

- The latest released version.
- The `main` branch.

Older beta releases may receive a security note or upgrade guidance, but fixes are normally made on `main` and released in the next patch version.

## Reporting a Vulnerability

Please do not open a public GitHub issue for a vulnerability.

Email security reports to:

**Tymour Kadry**  
<tymitaly@gmail.com>

Use the subject line `DictaPulse Security`.

Please include:

- A clear description of the issue.
- Steps to reproduce, proof of concept, or affected code path.
- Your operating system, desktop environment, display server, and DictaPulse version.
- Whether the issue affects local-only use, remote cleanup providers, packaging, model downloads, or text injection.
- Any logs that are relevant, with transcripts, API keys, tokens, private text, and personal data removed.

## What to Report

Please report issues such as:

- API keys, tokens, transcripts, or microphone recordings being exposed unintentionally.
- Secrets being written to config files, logs, crash output, screenshots, or issue reports.
- Remote cleanup providers receiving data when not explicitly enabled.
- Command injection, unsafe path handling, or unsafe environment variable handling.
- Unsafe model download behavior or integrity problems.
- Clipboard or text-injection behavior that exposes user content outside the selected target.
- Packaging behavior that installs files with unsafe permissions or unexpected system changes.
- Vulnerabilities in DictaPulse's integration with KGlobalAccel, KWin, `wtype`, `xdotool`, `ydotool`, QtKeychain, or related desktop services.

Please also report suspected dependency vulnerabilities if they are exploitable through DictaPulse.

## Response Expectations

You should receive an initial response within 7 days. I may ask for more information, confirm the impact, prepare a fix privately, and coordinate disclosure timing when appropriate.

When a fix is ready, the project may publish:

- A patched release.
- Release notes describing the impact and upgrade path.
- Credit to the reporter, unless you prefer to remain anonymous.

## Safe Research Guidelines

Please keep security research limited to your own systems and accounts.

Do not:

- Access, modify, or delete another person's data.
- Publish exploit details before the project has had time to respond.
- Submit reports containing real private transcripts, API keys, tokens, or recordings.
- Use DictaPulse project spaces to distribute malware, spyware, credential theft tooling, or license-violating builds.

Good-faith reports that follow this policy are welcome.
