#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
"""
detect-forks.py — scan public GitHub for copies of DictaPulse, including
rebranded ones that tried to strip the branding.

DictaPulse is GPL-3.0: forks are welcome, but they must stay open source and
preserve attribution (see NOTICE). This tool searches GitHub's code-search API
for the project's forensic markers. A rebrander has to find and scrub *every*
marker; they almost never do — so even a single hit establishes origin.

This is detection by scanning. It is NOT a beacon: the app phones nobody home.
You run this periodically; it reports suspect repositories.

Usage:
    GITHUB_TOKEN=ghp_xxx ./scripts/detect-forks.py
    GITHUB_TOKEN=ghp_xxx ./scripts/detect-forks.py --exclude silverhairfx

A token is required (GitHub code search needs auth). A read-only/public-repo
token is enough.
"""

import argparse
import json
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

# Forensic markers. Keep this list in sync with the private fingerprint manifest.
# Mix of the explicit provenance id and "organic" identifiers woven through the
# code that a rebrand is unlikely to fully change.
MARKERS = [
    "f33816f6-36ab-4c68-883e-f463fbb5cfe3",          # provenance UUID
    "DP-ORIGIN::ETK-Technologies",                    # provenance signature
    "org.dictapulse.ActiveWindow",                    # D-Bus interface name
    "dictapulse-activewindow",                        # KWin script id
    "kProvenanceId",                                  # constant name
    "ClaySurface",                                    # signature component name
    "ProfileContext",                                 # signature module name
]

API = "https://api.github.com/search/code"


def search(marker, token, exclude):
    q = f'"{marker}"'
    if exclude:
        q += f" -user:{exclude}"
    url = f"{API}?q={urllib.parse.quote(q)}&per_page=50"
    req = urllib.request.Request(url, headers={
        "Authorization": f"Bearer {token}",
        "Accept": "application/vnd.github.text-match+json",
        "User-Agent": "dictapulse-detect-forks",
    })
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            return json.load(r)
    except urllib.error.HTTPError as e:
        if e.code == 403:  # rate limit
            print("  ! rate-limited; waiting 60s…", file=sys.stderr)
            time.sleep(60)
            return search(marker, token, exclude)
        print(f"  ! HTTP {e.code} for marker {marker!r}", file=sys.stderr)
        return {"items": []}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exclude", default="silverhairfx",
                    help="GitHub user/org to exclude (your own account)")
    args = ap.parse_args()

    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        print("error: set GITHUB_TOKEN (a public-repo read token).", file=sys.stderr)
        sys.exit(2)

    hits = {}  # repo full_name -> set(markers)
    for marker in MARKERS:
        print(f"· searching: {marker}")
        data = search(marker, token, args.exclude)
        for item in data.get("items", []):
            repo = item["repository"]["full_name"]
            hits.setdefault(repo, set()).add(marker)
        time.sleep(2)  # be gentle with the code-search rate limit

    print("\n=== Suspect repositories ===")
    if not hits:
        print("None found. (No copies, or markers were fully scrubbed — widen MARKERS.)")
        return
    for repo, found in sorted(hits.items(), key=lambda kv: -len(kv[1])):
        print(f"\n  https://github.com/{repo}")
        print(f"    matched {len(found)}/{len(MARKERS)} markers:")
        for m in sorted(found):
            print(f"      • {m}")
    print("\nMore markers matched ⇒ stronger evidence of derivation. Compare git")
    print("history dates against this project's public commit timeline.")


if __name__ == "__main__":
    main()
