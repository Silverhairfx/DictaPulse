// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "ModelCatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace dictapulse {

namespace {
QVector<ModelInfo> buildCatalog()
{
    const QString base = QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/");
    QVector<ModelInfo> v;

    auto add = [&](const QString& id, const QString& name, const QString& file,
                   qint64 size, bool multi, const QString& family,
                   const QString& speed, const QString& accuracy, const QString& ram) {
        v.push_back({ id, name, file, base + file, size, multi, family, speed, accuracy, ram, {} });
    };

    add("tiny.en",    "Tiny (English-only)",  "ggml-tiny.en.bin",  77'691'000,   false, "tiny",   "very fast", "basic",     "~0.5 GB");
    add("tiny",       "Tiny (multilingual)",  "ggml-tiny.bin",     77'691'000,   true,  "tiny",   "very fast", "basic",     "~0.5 GB");
    add("base.en",    "Base (English-only)",  "ggml-base.en.bin",  147'951'000,  false, "base",   "fast",      "good",      "~0.7 GB");
    add("base",       "Base (multilingual)",  "ggml-base.bin",     147'951'000,  true,  "base",   "fast",      "good",      "~0.7 GB");
    add("small.en",   "Small (English-only)", "ggml-small.en.bin", 487'990'000,  false, "small",  "medium",    "very good", "~1.5 GB");
    add("small",      "Small (multilingual)", "ggml-small.bin",    487'990'000,  true,  "small",  "medium",    "very good", "~1.5 GB");
    add("medium.en",  "Medium (English-only)","ggml-medium.en.bin",1'533'763'000,false, "medium", "slow",      "excellent", "~3.5 GB");
    add("medium",     "Medium (multilingual)","ggml-medium.bin",   1'533'763'000,true,  "medium", "slow",      "excellent", "~3.5 GB");
    add("large-v3",   "Large v3 (multilingual)", "ggml-large-v3.bin", 3'100'000'000, true, "large", "very slow", "excellent", "~6 GB");
    add("large-v3-turbo", "Large v3 Turbo (multilingual)", "ggml-large-v3-turbo.bin", 1'620'000'000, true, "large-turbo", "medium", "excellent", "~4 GB");

    // Not hosted in the official ggerganov/whisper.cpp repository: no download URL.
    // It is offered once a copy is present in a model directory (see extraDirs()).
    v.push_back({ "large-v3-q8_0", "Large v3 Q8 (multilingual, local file)", "whisper-large-v3-q8_0.gguf",
                  QString(), 1'656'538'283, true, "large", "medium", "excellent", "~3 GB", {} });
    return v;
}
} // namespace

const QVector<ModelInfo>& ModelCatalog::all()
{
    static const QVector<ModelInfo> v = buildCatalog();
    return v;
}

const ModelInfo* ModelCatalog::find(const QString& id)
{
    for (const auto& m : all()) {
        if (m.id == id) return &m;
    }
    return nullptr;
}

QString ModelCatalog::downloadDir()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/models");
    QDir().mkpath(path);
    return path;
}

QStringList ModelCatalog::extraDirs()
{
    QStringList dirs;
    for (const QString& d : qEnvironmentVariable("DICTAPULSE_MODEL_DIRS").split(QLatin1Char(':'), Qt::SkipEmptyParts))
        dirs << d;
    QSettings s(QSettings::IniFormat, QSettings::UserScope,
                QCoreApplication::organizationName(), QCoreApplication::applicationName());
    for (const QString& d : s.value(QStringLiteral("models/extraDirs")).toStringList()) {
        if (!d.trimmed().isEmpty()) dirs << d.trimmed();
    }
    return dirs;
}

QString ModelCatalog::localPath(const ModelInfo& info)
{
    for (const QString& dir : extraDirs()) {
        const QString candidate = QDir(dir).filePath(info.filename);
        if (QFileInfo::exists(candidate)) return candidate;
    }
    return downloadDir() + "/" + info.filename;
}

} // namespace dictapulse
