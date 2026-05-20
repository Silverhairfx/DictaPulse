#pragma once

#include <QHash>
#include <QObject>
#include <QString>

namespace dictapulse {

class TextProcessor : public QObject {
    Q_OBJECT
public:
    struct Options {
        bool cleanup = true;
        bool capitalize = true;
        bool removeFillers = false;
        bool trailingSpace = true;
    };

    explicit TextProcessor(QObject* parent = nullptr);

    QString process(const QString& raw, const QString& language, const Options& opts) const;

    void setReplacements(const QHash<QString, QString>& map);

private:
    QHash<QString, QString> m_replacements;
};

} // namespace dictapulse
