#include "TextProcessor.h"

#include <QRegularExpression>
#include <QStringList>

namespace dictapulse {

TextProcessor::TextProcessor(QObject* parent)
    : QObject(parent)
{
}

void TextProcessor::setReplacements(const QHash<QString, QString>& map)
{
    m_replacements = map;
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

    if (!m_replacements.isEmpty()) {
        for (auto it = m_replacements.constBegin(); it != m_replacements.constEnd(); ++it) {
            const QRegularExpression re("\\b" + QRegularExpression::escape(it.key()) + "\\b",
                                         QRegularExpression::CaseInsensitiveOption);
            text.replace(re, it.value());
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

    if (opts.trailingSpace && !text.endsWith(' ')) text.append(' ');

    return text;
}

} // namespace dictapulse
