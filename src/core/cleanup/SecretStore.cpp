// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "SecretStore.h"

#ifdef DICTAPULSE_HAVE_KEYCHAIN
#include <qt6keychain/keychain.h>
#endif

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

#ifdef DICTAPULSE_HAVE_KEYCHAIN
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
#endif
}

void SecretStore::clearKey(const QString& id)
{
    m_cache.remove(id);
    emit changed(id);
#ifdef DICTAPULSE_HAVE_KEYCHAIN
    auto* job = new QKeychain::DeletePasswordJob(QString::fromLatin1(kService), this);
    job->setAutoDelete(true);
    job->setInsecureFallback(true);
    job->setKey(id);
    job->start();
#endif
}

void SecretStore::preload(const QStringList& ids)
{
#ifndef DICTAPULSE_HAVE_KEYCHAIN
    Q_UNUSED(ids)
    return;
#else
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
#endif
}

} // namespace dictapulse
