// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

namespace dictapulse {

// Provenance / origin fingerprint.
//
// DictaPulse is free software under the GPL-3.0. This constant is a forensic
// origin marker: it identifies this codebase as the original work of
// Tymour Kadry / ETK Technologies. It is surfaced in the About page and logged
// at startup so it lives in the shipped binary, and it is mirrored in several
// other places across the source and assets. Removing every copy from a
// rebranded fork is laborious and almost never complete - partial matches still
// establish origin. See scripts/detect-forks.py to scan public repos for it.
//
// This is provenance, not DRM: it phones nobody home and restricts nothing.
inline constexpr const char* kProvenanceId =
    "DP-ORIGIN::ETK-Technologies::Tymour-Kadry::f33816f6-36ab-4c68-883e-f463fbb5cfe3";

} // namespace dictapulse
