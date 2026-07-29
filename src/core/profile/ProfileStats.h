// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantList>

namespace dictapulse {

/// Tracks dictation usage so the Profile dashboard can show real metrics:
/// total words, sessions, top apps, and a daily trend. Persisted to the same
/// config store under a "stats/" group. This is also part of the user's
/// "profile" - the picture of how they actually dictate.
class ProfileStats : public QObject {
    Q_OBJECT
    Q_PROPERTY(int totalWords READ totalWords NOTIFY changed)
    Q_PROPERTY(int totalSessions READ totalSessions NOTIFY changed)

public:
    explicit ProfileStats(QObject* parent = nullptr);

    int totalWords() const;
    int totalSessions() const;

    /// Record one completed dictation. `appId` may be empty (unknown app).
    void record(int words, const QString& appId);

    /// Top apps by word count: [{ app, words }], descending, capped at `n`.
    Q_INVOKABLE QVariantList topApps(int n = 5) const;
    /// Daily word totals for the last `days` days (oldest→newest, zero-filled):
    /// [{ date: "MM-DD", words }].
    Q_INVOKABLE QVariantList dailySeries(int days = 14) const;
    /// Words dictated today.
    Q_INVOKABLE int wordsToday() const;

    Q_INVOKABLE void reset();

signals:
    void changed();

private:
    QSettings m_store;
};

} // namespace dictapulse
