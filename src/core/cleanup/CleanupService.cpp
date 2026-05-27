#include "CleanupService.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <cstdio>

namespace dictapulse {

CleanupService::CleanupService(QObject* parent)
    : QObject(parent)
    , m_net(new QNetworkAccessManager(this))
{
}

QString CleanupService::buildSystemPrompt(const Config& cfg, const QString& language) const
{
    QString sys = cfg.systemPrompt.trimmed();
    if (!language.isEmpty())
        sys += QStringLiteral("\n\nThe dictated text is in language code: %1. "
                              "Reply in that same language and script.").arg(language);
    if (!cfg.vocabulary.trimmed().isEmpty())
        sys += QStringLiteral("\n\nPrefer these user-specific spellings/terms when they "
                              "plausibly match what was said: %1").arg(cfg.vocabulary.trimmed());
    return sys;
}

void CleanupService::process(const QString& text, const QString& language, const Config& cfg)
{
    const QString sys = buildSystemPrompt(cfg, language);

    if (cfg.provider == QLatin1String("local")) {
        if (cfg.localEndpoint.trimmed().isEmpty()) {
            emit failed(tr("No local endpoint configured"));
            return;
        }
        postOpenAICompatible(cfg.localEndpoint, cfg.localModel, QString(), sys, text, cfg.timeoutMs);
        return;
    }

    if (cfg.provider == QLatin1String("remote")) {
        if (cfg.apiKey.isEmpty()) {
            emit failed(tr("No API key set for %1").arg(cfg.remoteProvider));
            return;
        }
        if (cfg.remoteProvider == QLatin1String("anthropic")) {
            postAnthropic(cfg.remoteModel, cfg.apiKey, sys, text, cfg.timeoutMs);
        } else if (cfg.remoteProvider == QLatin1String("openai")) {
            postOpenAICompatible(QStringLiteral("https://api.openai.com/v1"),
                                 cfg.remoteModel, cfg.apiKey, sys, text, cfg.timeoutMs);
        } else { // custom OpenAI-compatible
            if (cfg.remoteEndpoint.trimmed().isEmpty()) {
                emit failed(tr("No custom endpoint configured"));
                return;
            }
            postOpenAICompatible(cfg.remoteEndpoint, cfg.remoteModel, cfg.apiKey,
                                 sys, text, cfg.timeoutMs);
        }
        return;
    }

    emit failed(tr("Unknown cleanup provider: %1").arg(cfg.provider));
}

void CleanupService::postOpenAICompatible(const QString& base, const QString& model,
                                          const QString& apiKey, const QString& sys,
                                          const QString& user, int timeoutMs)
{
    QString url = base.trimmed();
    while (url.endsWith('/')) url.chop(1);
    url += QStringLiteral("/chat/completions");

    QJsonObject sysMsg{{"role", "system"}, {"content", sys}};
    QJsonObject userMsg{{"role", "user"}, {"content", user}};
    QJsonObject body{
        {"model", model.isEmpty() ? QStringLiteral("local-model") : model},
        {"messages", QJsonArray{sysMsg, userMsg}},
        {"temperature", 0.2},
        {"stream", false},
    };

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setTransferTimeout(timeoutMs);
    if (!apiKey.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    std::fprintf(stderr, "[DictaPulse] cleanup POST %s model='%s'\n",
                 qUtf8Printable(url), qUtf8Printable(model));
    std::fflush(stderr);

    QNetworkReply* reply = m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleReply(reply, false); });
}

void CleanupService::postAnthropic(const QString& model, const QString& apiKey,
                                   const QString& sys, const QString& user, int timeoutMs)
{
    QJsonObject body{
        {"model", model.isEmpty() ? QStringLiteral("claude-haiku-4-5") : model},
        {"max_tokens", 2048},
        {"system", sys},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", user}}}},
    };

    QNetworkRequest req{QUrl(QStringLiteral("https://api.anthropic.com/v1/messages"))};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", apiKey.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");
    req.setTransferTimeout(timeoutMs);

    std::fprintf(stderr, "[DictaPulse] cleanup POST anthropic model='%s'\n",
                 qUtf8Printable(model));
    std::fflush(stderr);

    QNetworkReply* reply = m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { handleReply(reply, true); });
}

void CleanupService::handleReply(QNetworkReply* reply, bool anthropic)
{
    reply->deleteLater();
    const QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        QString detail = reply->errorString();
        // Surface the API's own error message when present — far more useful.
        const QJsonObject obj = QJsonDocument::fromJson(data).object();
        if (obj.contains("error")) {
            const QJsonValue e = obj.value("error");
            const QString msg = e.isObject() ? e.toObject().value("message").toString()
                                             : e.toString();
            if (!msg.isEmpty()) detail = msg;
        }
        emit failed(detail);
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    QString out;
    if (anthropic) {
        const QJsonArray content = doc.object().value("content").toArray();
        for (const QJsonValue& v : content) {
            if (v.toObject().value("type").toString() == QLatin1String("text"))
                out += v.toObject().value("text").toString();
        }
    } else {
        const QJsonArray choices = doc.object().value("choices").toArray();
        if (!choices.isEmpty())
            out = choices.first().toObject().value("message").toObject().value("content").toString();
    }

    out = out.trimmed();
    if (out.isEmpty()) {
        emit failed(tr("LLM returned an empty response"));
        return;
    }
    emit cleaned(out);
}

} // namespace dictapulse
