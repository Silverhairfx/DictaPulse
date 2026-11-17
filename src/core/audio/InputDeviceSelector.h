// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

namespace dictapulse {

struct InputCandidate {
    QByteArray id;
    QString description;
    bool isDefault = false;
};

/// Chooses a capture device without touching the system's default input.
///
/// Order: an explicit device id from settings, then the first device whose id or
/// description contains one of the preferred patterns (case-insensitive, e.g.
/// "boya"), then the system default. Monitor sources are never chosen.
/// Returns the index into `devices`, or -1 when there is no usable device.
int pickInputDevice(const QVector<InputCandidate>& devices,
                    const QString& explicitId,
                    const QStringList& preferredPatterns);

QStringList parsePreferredPatterns(const QString& csv);

} // namespace dictapulse
