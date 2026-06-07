// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QString>
#include <QVector>

namespace dictapulse {

class TextProcessor : public QObject {
    Q_OBJECT
public:
    struct Options {
        bool cleanup = true;
        bool capitalize = true;
        bool removeFillers = false;
        bool trailingSpace = true;
        bool smartLists = false;
    };

    // A single personal-dictionary rule. `lang` empty = applies to all
    // languages, otherwise matched by prefix (e.g. "en" matches "en").
    struct Replacement {
        QString from;
        QString to;
        bool caseSensitive = false;
        bool wholeWord = true;
        QString lang;
    };

    explicit TextProcessor(QObject* parent = nullptr);

    QString process(const QString& raw, const QString& language, const Options& opts) const;

    void setReplacements(const QVector<Replacement>& list);

private:
    QVector<Replacement> m_replacements;
};

} // namespace dictapulse
