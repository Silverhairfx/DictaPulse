// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "InputDeviceSelector.h"

namespace dictapulse {

QStringList parsePreferredPatterns(const QString& csv)
{
    QStringList out;
    for (const QString& part : csv.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const QString p = part.trimmed().toLower();
        if (!p.isEmpty()) out << p;
    }
    return out;
}

static bool isMonitor(const InputCandidate& c)
{
    return c.id.contains(".monitor") || c.description.startsWith(QStringLiteral("Monitor of "));
}

int pickInputDevice(const QVector<InputCandidate>& devices,
                    const QString& explicitId,
                    const QStringList& preferredPatterns)
{
    if (!explicitId.isEmpty()) {
        for (int i = 0; i < devices.size(); ++i) {
            if (devices[i].id == explicitId.toUtf8()) return i;
        }
    }
    for (const QString& pattern : preferredPatterns) {
        for (int i = 0; i < devices.size(); ++i) {
            if (isMonitor(devices[i])) continue;
            if (QString::fromUtf8(devices[i].id).toLower().contains(pattern)
                || devices[i].description.toLower().contains(pattern)) {
                return i;
            }
        }
    }
    for (int i = 0; i < devices.size(); ++i) {
        if (devices[i].isDefault) return i;
    }
    return devices.isEmpty() ? -1 : 0;
}

} // namespace dictapulse
