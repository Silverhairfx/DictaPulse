#include "ModelCatalog.h"

#include <QDir>
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

QString ModelCatalog::localPath(const ModelInfo& info)
{
    return downloadDir() + "/" + info.filename;
}

} // namespace dictapulse
