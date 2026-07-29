// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "ProfileContext.h"

#include "app/Settings.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

namespace dictapulse {
namespace ProfileContext {

namespace {
QJsonArray parseArray(const QString& json)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    return doc.isArray() ? doc.array() : QJsonArray{};
}

// Split a free-form jargon blob (newlines and/or commas) into clean terms.
QStringList splitTerms(const QString& blob)
{
    QStringList out;
    const QStringList parts = blob.split(QRegularExpression(R"([,\n])"), Qt::SkipEmptyParts);
    for (const QString& p : parts) {
        const QString t = p.trimmed();
        if (!t.isEmpty()) out << t;
    }
    return out;
}

QString toneInstruction(const QString& tone)
{
    if (tone == "formal")       return QStringLiteral("Write in a polished, formal tone.");
    if (tone == "casual")       return QStringLiteral("Write in a relaxed, casual tone.");
    if (tone == "enthusiastic") return QStringLiteral("Write in a warm, enthusiastic tone.");
    if (tone == "technical")    return QStringLiteral("Write in a precise, technical tone.");
    if (tone == "neutral")      return QStringLiteral("Write in a clear, neutral tone.");
    return {};
}
} // namespace

QVector<TextProcessor::Replacement> replacements(const Settings* s)
{
    QVector<TextProcessor::Replacement> rules;

    // Dictionary entries.
    for (const QJsonValue& v : parseArray(s->dictionary())) {
        const QJsonObject o = v.toObject();
        TextProcessor::Replacement r;
        r.from = o.value("from").toString();
        r.to = o.value("to").toString();
        r.caseSensitive = o.value("caseSensitive").toBool(false);
        r.wholeWord = o.value("wholeWord").toBool(true);
        r.lang = o.value("lang").toString();
        if (!r.from.isEmpty()) rules.append(r);
    }

    // Voice templates: a spoken cue expands to its full text. Phrase-matched,
    // case-insensitive - applied alongside the dictionary.
    for (const QJsonValue& v : parseArray(s->voiceTemplates())) {
        const QJsonObject o = v.toObject();
        TextProcessor::Replacement r;
        r.from = o.value("cue").toString().trimmed();
        r.to = o.value("expansion").toString();
        r.caseSensitive = false;
        r.wholeWord = true;
        if (!r.from.isEmpty() && !r.to.isEmpty()) rules.append(r);
    }

    return rules;
}

QString whisperVocabulary(const Settings* s)
{
    if (!s->dictionaryBias()) return {};
    QStringList terms;
    QSet<QString> seen;
    auto add = [&](const QString& t) {
        const QString v = t.trimmed();
        if (v.isEmpty() || seen.contains(v)) return;
        seen.insert(v);
        terms << v;
    };

    for (const QJsonValue& v : parseArray(s->dictionary()))
        add(v.toObject().value("to").toString());
    for (const QJsonValue& v : parseArray(s->voiceTemplates()))
        add(v.toObject().value("cue").toString());
    if (s->devModeEnabled())
        for (const QString& t : splitTerms(s->devJargon())) add(t);

    if (terms.isEmpty()) return {};
    QString prompt = terms.join(QStringLiteral(", "));
    if (prompt.size() > 800) prompt = prompt.left(800);
    return prompt;
}

QString systemPromptAugmentation(const Settings* s)
{
    QStringList lines;

    // Tone / style.
    const QString tone = toneInstruction(s->toneStyle());
    if (!tone.isEmpty()) lines << tone;

    // Refine behaviors.
    if (s->refineAutoPunctuation())
        lines << QStringLiteral("Add natural punctuation and capitalization from the phrasing.");
    if (s->removeFillerWords())
        lines << QStringLiteral("Remove filler words (um, uh, like, you know).");
    if (s->refineSmartLists())
        lines << QStringLiteral("When the speaker enumerates items, format them as a clean numbered or bulleted list.");
    if (s->refineBacktrack())
        lines << QStringLiteral("Apply spoken self-corrections: when the speaker changes their mind (e.g. \"at 2, actually 3\"), keep only the corrected version.");

    // Developer mode.
    if (s->devModeEnabled()) {
        lines << QStringLiteral("This is technical/developer dictation: preserve code verbatim - keep camelCase, snake_case, CLI commands, file paths and exact spacing; do not capitalize or punctuate code tokens.");
        const QStringList jargon = splitTerms(s->devJargon());
        if (!jargon.isEmpty())
            lines << QStringLiteral("Recognize and spell these technical terms correctly: %1.")
                         .arg(jargon.join(QStringLiteral(", ")));
    }

    // Dictionary - explicit spelling map so the LLM honors it too.
    QStringList dictPairs;
    for (const QJsonValue& v : parseArray(s->dictionary())) {
        const QJsonObject o = v.toObject();
        const QString from = o.value("from").toString().trimmed();
        const QString to = o.value("to").toString().trimmed();
        if (!from.isEmpty() && !to.isEmpty())
            dictPairs << QStringLiteral("\"%1\"→\"%2\"").arg(from, to);
    }
    if (!dictPairs.isEmpty())
        lines << QStringLiteral("Apply the user's preferred spellings: %1.").arg(dictPairs.join(QStringLiteral(", ")));

    // Voice templates.
    QStringList tplPairs;
    for (const QJsonValue& v : parseArray(s->voiceTemplates())) {
        const QJsonObject o = v.toObject();
        const QString cue = o.value("cue").toString().trimmed();
        const QString exp = o.value("expansion").toString().trimmed();
        if (!cue.isEmpty() && !exp.isEmpty())
            tplPairs << QStringLiteral("\"%1\"").arg(cue);
    }
    if (!tplPairs.isEmpty())
        lines << QStringLiteral("If the text is exactly one of these cues, expand it to the user's saved template: %1.").arg(tplPairs.join(QStringLiteral(", ")));

    if (lines.isEmpty()) return {};
    return QStringLiteral("\n\nUser profile preferences:\n- ") + lines.join(QStringLiteral("\n- "));
}

} // namespace ProfileContext
} // namespace dictapulse
