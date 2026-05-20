#pragma once

#include "ModelCatalog.h"

#include <QAbstractListModel>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

namespace dictapulse {

class ModelManager : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        FamilyRole,
        SizeRole,
        SizeHumanRole,
        MultilingualRole,
        SpeedRole,
        AccuracyRole,
        RamRole,
        InstalledRole,
        DownloadingRole,
        ProgressRole,
        LocalPathRole,
    };

    explicit ModelManager(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void download(const QString& id);
    Q_INVOKABLE void cancel(const QString& id);
    Q_INVOKABLE bool remove(const QString& id);
    Q_INVOKABLE QString localPathFor(const QString& id) const;
    Q_INVOKABLE bool isInstalled(const QString& id) const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString defaultInstalledId() const;

signals:
    void downloadStarted(const QString& id);
    void downloadFinished(const QString& id, const QString& localPath);
    void downloadFailed(const QString& id, const QString& error);
    void downloadProgress(const QString& id, qint64 received, qint64 total);

private:
    struct ActiveDownload {
        QNetworkReply* reply = nullptr;
        QString tempPath;
        qint64 received = 0;
        qint64 total = 0;
    };

    int rowFor(const QString& id) const;
    void emitRowChanged(const QString& id);

    QNetworkAccessManager m_net;
    QHash<QString, ActiveDownload> m_active;
};

} // namespace dictapulse
