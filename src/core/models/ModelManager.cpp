#include "ModelManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QNetworkRequest>

namespace dictapulse {

ModelManager::ModelManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ModelManager::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ModelCatalog::all().size();
}

QHash<int, QByteArray> ModelManager::roleNames() const
{
    return {
        { IdRole, "modelId" },
        { NameRole, "name" },
        { FamilyRole, "family" },
        { SizeRole, "sizeBytes" },
        { SizeHumanRole, "sizeHuman" },
        { MultilingualRole, "multilingual" },
        { SpeedRole, "speed" },
        { AccuracyRole, "accuracy" },
        { RamRole, "minRam" },
        { InstalledRole, "installed" },
        { DownloadingRole, "downloading" },
        { ProgressRole, "progress" },
        { LocalPathRole, "localPath" },
    };
}

QVariant ModelManager::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto& list = ModelCatalog::all();
    if (index.row() < 0 || index.row() >= list.size()) return {};
    const ModelInfo& m = list[index.row()];
    const QString local = ModelCatalog::localPath(m);
    const auto it = m_active.find(m.id);

    switch (role) {
    case IdRole: return m.id;
    case NameRole: return m.name;
    case FamilyRole: return m.family;
    case SizeRole: return m.sizeBytes;
    case SizeHumanRole: return QLocale().formattedDataSize(m.sizeBytes);
    case MultilingualRole: return m.multilingual;
    case SpeedRole: return m.speed;
    case AccuracyRole: return m.accuracy;
    case RamRole: return m.minRam;
    case InstalledRole: return QFileInfo::exists(local);
    case DownloadingRole: return it != m_active.end();
    case ProgressRole:
        if (it != m_active.end() && it->total > 0) {
            return static_cast<double>(it->received) / static_cast<double>(it->total);
        }
        return 0.0;
    case LocalPathRole: return local;
    }
    return {};
}

int ModelManager::rowFor(const QString& id) const
{
    const auto& list = ModelCatalog::all();
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].id == id) return i;
    }
    return -1;
}

void ModelManager::emitRowChanged(const QString& id)
{
    const int row = rowFor(id);
    if (row < 0) return;
    emit dataChanged(index(row), index(row));
}

QString ModelManager::localPathFor(const QString& id) const
{
    const auto* info = ModelCatalog::find(id);
    return info ? ModelCatalog::localPath(*info) : QString();
}

bool ModelManager::isInstalled(const QString& id) const
{
    const QString p = localPathFor(id);
    return !p.isEmpty() && QFileInfo::exists(p);
}

void ModelManager::refresh()
{
    if (rowCount() == 0) return;
    emit dataChanged(index(0), index(rowCount() - 1));
}

QString ModelManager::defaultInstalledId() const
{
    for (const auto& m : ModelCatalog::all()) {
        if (QFileInfo::exists(ModelCatalog::localPath(m))) return m.id;
    }
    return {};
}

void ModelManager::download(const QString& id)
{
    if (m_active.contains(id)) return;
    const auto* info = ModelCatalog::find(id);
    if (!info) {
        emit downloadFailed(id, tr("Unknown model id"));
        return;
    }
    const QString local = ModelCatalog::localPath(*info);
    QDir().mkpath(QFileInfo(local).absolutePath());

    QNetworkRequest req((QUrl(info->url)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DictaPulse/0.1"));

    QNetworkReply* reply = m_net.get(req);
    ActiveDownload d;
    d.reply = reply;
    d.tempPath = local + ".part";
    m_active.insert(id, d);
    emit downloadStarted(id);
    emitRowChanged(id);

    auto* tempFile = new QFile(d.tempPath, reply);
    if (!tempFile->open(QIODevice::WriteOnly)) {
        emit downloadFailed(id, tr("Cannot open %1 for writing").arg(d.tempPath));
        m_active.remove(id);
        reply->abort();
        reply->deleteLater();
        emitRowChanged(id);
        return;
    }

    connect(reply, &QNetworkReply::readyRead, this, [this, id, reply, tempFile]() {
        tempFile->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this, id](qint64 r, qint64 t) {
                auto it = m_active.find(id);
                if (it == m_active.end()) return;
                it->received = r;
                it->total = t;
                emit downloadProgress(id, r, t);
                emitRowChanged(id);
            });
    connect(reply, &QNetworkReply::finished, this, [this, id, reply, tempFile]() {
        auto it = m_active.find(id);
        if (it == m_active.end()) {
            reply->deleteLater();
            return;
        }
        tempFile->write(reply->readAll());
        tempFile->flush();
        tempFile->close();

        const QString tempPath = it->tempPath;
        m_active.erase(it);

        const bool aborted = reply->error() == QNetworkReply::OperationCanceledError;
        if (reply->error() != QNetworkReply::NoError) {
            QFile::remove(tempPath);
            if (!aborted) emit downloadFailed(id, reply->errorString());
            emitRowChanged(id);
            reply->deleteLater();
            return;
        }
        const QString finalPath = QString(tempPath).chopped(5); // strip .part
        QFile::remove(finalPath);
        if (!QFile::rename(tempPath, finalPath)) {
            emit downloadFailed(id, tr("Failed to rename downloaded file"));
            emitRowChanged(id);
            reply->deleteLater();
            return;
        }
        emit downloadFinished(id, finalPath);
        emitRowChanged(id);
        reply->deleteLater();
    });
}

void ModelManager::cancel(const QString& id)
{
    auto it = m_active.find(id);
    if (it == m_active.end()) return;
    it->reply->abort();
    QFile::remove(it->tempPath);
    m_active.erase(it);
    emitRowChanged(id);
}

bool ModelManager::remove(const QString& id)
{
    const QString p = localPathFor(id);
    if (p.isEmpty() || !QFile::exists(p)) return false;
    const bool ok = QFile::remove(p);
    if (ok) emitRowChanged(id);
    return ok;
}

} // namespace dictapulse
