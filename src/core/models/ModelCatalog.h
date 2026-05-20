#pragma once

#include <QString>
#include <QVector>

namespace dictapulse {

struct ModelInfo {
    QString id;
    QString name;
    QString filename;
    QString url;
    qint64 sizeBytes = 0;
    bool multilingual = false;
    QString family;        // tiny/base/small/medium/large
    QString speed;         // very fast/fast/medium/slow/very slow
    QString accuracy;      // basic/good/very good/excellent
    QString minRam;        // recommended free RAM
    QStringList languages; // empty = all whisper-supported
};

class ModelCatalog {
public:
    static const QVector<ModelInfo>& all();
    static const ModelInfo* find(const QString& id);
    static QString downloadDir();
    static QString localPath(const ModelInfo& info);
};

} // namespace dictapulse
