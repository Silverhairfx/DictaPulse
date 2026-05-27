#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

namespace dictapulse {

// Thin wrapper over QtKeychain (KWallet / libsecret on Linux) for API keys.
// Secrets live in the OS secure store, never in QSettings. After an async read
// they're mirrored in an in-memory cache so request-time access is synchronous.
class SecretStore : public QObject {
    Q_OBJECT
public:
    explicit SecretStore(QObject* parent = nullptr);

    // Synchronous cache access (populated by preload() / setKey()).
    QString key(const QString& id) const { return m_cache.value(id); }
    Q_INVOKABLE bool hasKey(const QString& id) const { return !m_cache.value(id).isEmpty(); }

    // Persist to the secure store (async); the cache updates immediately so the
    // value is usable right away.
    Q_INVOKABLE void setKey(const QString& id, const QString& value);
    Q_INVOKABLE void clearKey(const QString& id);

    // Load the given ids from the secure store into the cache. Call at startup.
    void preload(const QStringList& ids);

signals:
    void changed(const QString& id);

private:
    QHash<QString, QString> m_cache;
};

} // namespace dictapulse
