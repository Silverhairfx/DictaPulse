// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QString>

namespace dictapulse {

/// Handles text insertion on Wayland (wtype) and X11 (xdotool fallback), with
/// clipboard-based output as the safe fallback path.
class TextInjector : public QObject {
    Q_OBJECT
public:
    enum class Result {
        Inserted,
        ClipboardOnly,
        Failed,
    };

    explicit TextInjector(QObject* parent = nullptr);

    Result inject(const QString& text, const QString& mode, bool clipboardFallback);

    static bool needsPasteRoute(const QString& text);

    bool wtypeAvailable() const { return m_hasWtype; }
    bool xdotoolAvailable() const { return m_hasXdotool; }
    bool ydotoolAvailable() const { return m_hasYdotool; }
    bool isWayland() const { return m_isWayland; }

private:
    bool typeViaWtype(const QString& text);
    bool typeViaXdotool(const QString& text);
    bool typeViaYdotool(const QString& text);
    void copyToClipboard(const QString& text);
    bool sendPasteShortcut();

    // KDE keyboard-layout switching (DBus) so paste keysyms aren't mangled
    // by a non-Latin active layout.
    static int currentKbLayout();
    static int findLatinLayout();
    static bool setKbLayout(int index);

    bool m_hasWtype = false;
    bool m_hasXdotool = false;
    bool m_hasYdotool = false;
    bool m_isWayland = false;
};

} // namespace dictapulse
