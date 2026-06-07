// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "ProfileStats.h"

#include <QCoreApplication>
#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

#include <algorithm>

namespace dictapulse {

namespace {
constexpr auto kTotalWords = "stats/totalWords";
constexpr auto kTotalSessions = "stats/totalSessions";
constexpr auto kPerApp = "stats/perApp";   // JSON object app→words
constexpr auto kDaily = "stats/daily";     // JSON object yyyy-MM-dd→words

QJsonObject readObj(QSettings& s, const char* key)
{
    return QJsonDocument::fromJson(s.value(key, "{}").toString().toUtf8()).object();
}
void writeObj(QSettings& s, const char* key, const QJsonObject& o)
{
    s.setValue(key, QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
}
} // namespace

ProfileStats::ProfileStats(QObject* parent)
    : QObject(parent)
    , m_store(QSettings::IniFormat, QSettings::UserScope,
              QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
}

int ProfileStats::totalWords() const { return m_store.value(kTotalWords, 0).toInt(); }
int ProfileStats::totalSessions() const { return m_store.value(kTotalSessions, 0).toInt(); }

void ProfileStats::record(int words, const QString& appId)
{
    if (words <= 0) return;

    m_store.setValue(kTotalWords, totalWords() + words);
    m_store.setValue(kTotalSessions, totalSessions() + 1);

    const QString app = appId.isEmpty() ? QStringLiteral("unknown") : appId.toLower();
    QJsonObject perApp = readObj(m_store, kPerApp);
    perApp[app] = perApp.value(app).toInt() + words;
    writeObj(m_store, kPerApp, perApp);

    const QString today = QDate::currentDate().toString(Qt::ISODate);
    QJsonObject daily = readObj(m_store, kDaily);
    daily[today] = daily.value(today).toInt() + words;
    // Prune anything older than ~90 days to keep the blob small.
    const QString cutoff = QDate::currentDate().addDays(-90).toString(Qt::ISODate);
    for (const QString& k : daily.keys())
        if (k < cutoff) daily.remove(k);
    writeObj(m_store, kDaily, daily);

    emit changed();
}

QVariantList ProfileStats::topApps(int n) const
{
    QSettings s(QSettings::IniFormat, QSettings::UserScope,
                QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QJsonObject perApp = readObj(s, kPerApp);
    QList<QPair<QString, int>> items;
    for (auto it = perApp.begin(); it != perApp.end(); ++it)
        items.append({ it.key(), it.value().toInt() });
    std::sort(items.begin(), items.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    QVariantList out;
    for (int i = 0; i < items.size() && i < n; ++i) {
        QVariantMap m;
        m["app"] = items[i].first;
        m["words"] = items[i].second;
        out.append(m);
    }
    return out;
}

QVariantList ProfileStats::dailySeries(int days) const
{
    QSettings s(QSettings::IniFormat, QSettings::UserScope,
                QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QJsonObject daily = readObj(s, kDaily);
    QVariantList out;
    const QDate today = QDate::currentDate();
    for (int i = days - 1; i >= 0; --i) {
        const QDate d = today.addDays(-i);
        QVariantMap m;
        m["date"] = d.toString("MM-dd");
        m["words"] = daily.value(d.toString(Qt::ISODate)).toInt();
        out.append(m);
    }
    return out;
}

int ProfileStats::wordsToday() const
{
    QSettings s(QSettings::IniFormat, QSettings::UserScope,
                QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QJsonObject daily = readObj(s, kDaily);
    return daily.value(QDate::currentDate().toString(Qt::ISODate)).toInt();
}

void ProfileStats::reset()
{
    m_store.remove("stats");
    m_store.sync();
    emit changed();
}

} // namespace dictapulse
