// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

namespace dictapulse {

// Async transcript cleanup via an LLM. Handles the network ("local" =
// OpenAI-compatible endpoint like Ollama / LM Studio; "remote" = Anthropic /
// OpenAI / custom). The trivial providers ("none" / "rules") are handled by the
// caller (Controller + TextProcessor), not here. One request in flight at a
// time — dictation is sequential.
class CleanupService : public QObject {
    Q_OBJECT
public:
    struct Config {
        QString provider;        // "local" | "remote"
        QString localEndpoint;   // base URL incl. /v1, e.g. http://localhost:1234/v1
        QString localModel;
        QString remoteProvider;  // "anthropic" | "openai" | "custom"
        QString remoteModel;
        QString remoteEndpoint;  // base URL incl. /v1 (custom OpenAI-compatible)
        QString apiKey;
        QString systemPrompt;
        QString vocabulary;      // optional terms to bias toward (filled in M4)
        int timeoutMs = 20000;
    };

    explicit CleanupService(QObject* parent = nullptr);

    void process(const QString& text, const QString& language, const Config& cfg);

signals:
    void cleaned(const QString& text);
    void failed(const QString& error);

private:
    QString buildSystemPrompt(const Config& cfg, const QString& language) const;
    void postOpenAICompatible(const QString& base, const QString& model,
                              const QString& apiKey, const QString& sys,
                              const QString& user, int timeoutMs);
    void postAnthropic(const QString& model, const QString& apiKey,
                       const QString& sys, const QString& user, int timeoutMs);
    void handleReply(QNetworkReply* reply, bool anthropic);

    QNetworkAccessManager* m_net;
};

} // namespace dictapulse
