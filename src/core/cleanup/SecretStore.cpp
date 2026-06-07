// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "SecretStore.h"

#include <qt6keychain/keychain.h>

#include <QDebug>

namespace dictapulse {

namespace {
constexpr auto kService = "DictaPulse";
}

SecretStore::SecretStore(QObject* parent)
    : QObject(parent)
{
}

void SecretStore::setKey(const QString& id, const QString& value)
{
    // Optimistic cache update so callers don't have to wait for the write.
    if (value.isEmpty()) {
        m_cache.remove(id);
    } else {
        m_cache.insert(id, value);
    }
    emit changed(id);

    if (value.isEmpty()) {
        clearKey(id);
        return;
    }

    auto* job = new QKeychain::WritePasswordJob(QString::fromLatin1(kService), this);
    job->setAutoDelete(true);
    job->setInsecureFallback(true);  // tolerate boxes without KWallet/libsecret
    job->setKey(id);
    job->setTextData(value);
    connect(job, &QKeychain::Job::finished, this, [id](QKeychain::Job* j) {
        if (j->error())
            qWarning() << "[DictaPulse] keychain write failed for" << id << ":" << j->errorString();
    });
    job->start();
}

void SecretStore::clearKey(const QString& id)
{
    m_cache.remove(id);
    emit changed(id);
    auto* job = new QKeychain::DeletePasswordJob(QString::fromLatin1(kService), this);
    job->setAutoDelete(true);
    job->setInsecureFallback(true);
    job->setKey(id);
    job->start();
}

void SecretStore::preload(const QStringList& ids)
{
    for (const QString& id : ids) {
        auto* job = new QKeychain::ReadPasswordJob(QString::fromLatin1(kService), this);
        job->setAutoDelete(true);
        job->setInsecureFallback(true);
        job->setKey(id);
        connect(job, &QKeychain::Job::finished, this, [this, id](QKeychain::Job* j) {
            auto* rj = static_cast<QKeychain::ReadPasswordJob*>(j);
            if (!rj->error() && !rj->textData().isEmpty()) {
                m_cache.insert(id, rj->textData());
                emit changed(id);
            }
        });
        job->start();
    }
}

} // namespace dictapulse
