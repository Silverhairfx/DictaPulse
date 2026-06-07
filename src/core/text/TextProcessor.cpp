// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "TextProcessor.h"

#include <QRegularExpression>
#include <QStringList>

namespace dictapulse {

TextProcessor::TextProcessor(QObject* parent)
    : QObject(parent)
{
}

void TextProcessor::setReplacements(const QVector<Replacement>& list)
{
    m_replacements = list;
}

QString TextProcessor::process(const QString& raw, const QString& language, const Options& opts) const
{
    QString text = raw.trimmed();
    if (text.isEmpty()) return text;

    static const QRegularExpression spaceBeforePunct(R"(\s+([.,!?;:]))");
    static const QRegularExpression multiSpace(R"(\s{2,})");

    if (opts.cleanup) {
        text.replace(spaceBeforePunct, QStringLiteral("\\1"));
        text.replace(multiSpace, QStringLiteral(" "));
    }

    // Arabic punctuation layer: Whisper emits Latin punctuation even for Arabic.
    // Map to the Arabic forms and tighten spacing. Letter-level corrections are
    // left to the LLM cleanup providers — touching them here risks changing the
    // user's intended spelling.
    if (opts.cleanup && language.startsWith(QLatin1String("ar"))) {
        text.replace(QChar('?'), QChar(0x061F));  // ؟ Arabic question mark
        text.replace(QChar(','), QChar(0x060C));  // ، Arabic comma
        text.replace(QChar(';'), QChar(0x061B));  // ؛ Arabic semicolon
        static const QRegularExpression spaceBeforeAr(
            QStringLiteral("\\s+([\\x{060C}\\x{061B}\\x{061F}])"));
        text.replace(spaceBeforeAr, QStringLiteral("\\1"));
    }

    if (opts.removeFillers && (language.startsWith("en") || language.isEmpty())) {
        static const QStringList fillers = { "um", "uh", "erm", "uhh", "umm", "mm", "hmm" };
        const QStringList words = text.split(QRegularExpression(R"(\s+)"), Qt::SkipEmptyParts);
        QStringList kept;
        kept.reserve(words.size());
        for (const QString& w : words) {
            const QString lower = w.toLower();
            QString stripped = lower;
            stripped.remove(QRegularExpression(R"([^a-z])"));
            if (fillers.contains(stripped)) continue;
            kept.append(w);
        }
        text = kept.join(' ');
    }

    // Smart lists: when the speaker enumerates inline ("…for 1. apples 2. bananas
    // 3. oranges"), break each numbered item onto its own line. Conservative —
    // only fires when there are at least two "<n>. " or "<n>) " markers so we
    // don't mangle ordinary numbers like "section 1. introduction".
    if (opts.smartLists) {
        static const QRegularExpression marker(R"((\d+)[.\)]\s+)");
        int count = 0;
        for (auto it = marker.globalMatch(text); it.hasNext(); it.next()) ++count;
        if (count >= 2) {
            // Put a newline before every marker after the first; trim the lead-in
            // up to the first marker so the list starts clean.
            const QRegularExpressionMatch first = marker.match(text);
            QString head = text.left(first.capturedStart()).trimmed();
            QString body = text.mid(first.capturedStart());
            body.replace(QRegularExpression(R"(\s*(\d+)[.\)]\s+)"), QStringLiteral("\n\\1. "));
            text = (head.isEmpty() ? QString() : head + QStringLiteral("\n")) + body.trimmed();
        }
    }

    if (opts.capitalize && !text.isEmpty()) {
        text[0] = text[0].toUpper();
        static const QRegularExpression sentenceEnd(R"(([.!?]\s+)([a-z]))");
        QRegularExpressionMatchIterator it = sentenceEnd.globalMatch(text);
        QString out;
        out.reserve(text.size());
        int last = 0;
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            out.append(text.mid(last, m.capturedStart(2) - last));
            out.append(m.captured(2).toUpper());
            last = m.capturedEnd(2);
        }
        out.append(text.mid(last));
        text = out;
    }

    // Personal dictionary — applied LAST so the user's exact target casing wins
    // over the sentence-capitalization pass above. Each rule is filtered by
    // language (empty = all, else prefix match), and honors its own
    // case-sensitivity and whole-word flags.
    for (const Replacement& r : m_replacements) {
        if (r.from.isEmpty()) continue;
        if (!r.lang.isEmpty() && !language.startsWith(r.lang, Qt::CaseInsensitive)) continue;

        QString pattern = QRegularExpression::escape(r.from);
        if (r.wholeWord) pattern = QStringLiteral("\\b") + pattern + QStringLiteral("\\b");
        QRegularExpression::PatternOptions popts = QRegularExpression::UseUnicodePropertiesOption;
        if (!r.caseSensitive) popts |= QRegularExpression::CaseInsensitiveOption;
        const QRegularExpression re(pattern, popts);
        if (re.isValid()) text.replace(re, r.to);
    }

    if (opts.trailingSpace && !text.endsWith(' ')) text.append(' ');

    return text;
}

} // namespace dictapulse
