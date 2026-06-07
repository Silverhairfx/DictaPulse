// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace dictapulse {

/// Best-effort runtime hardware detection — returns a struct of human-readable
/// strings plus booleans for available acceleration APIs.
class HardwareInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString cpuModel READ cpuModel CONSTANT)
    Q_PROPERTY(int cpuThreads READ cpuThreads CONSTANT)
    Q_PROPERTY(QString ramHuman READ ramHuman CONSTANT)
    Q_PROPERTY(QString gpuVendor READ gpuVendor CONSTANT)
    Q_PROPERTY(QString gpuModel READ gpuModel CONSTANT)
    Q_PROPERTY(bool hasVulkan READ hasVulkan CONSTANT)
    Q_PROPERTY(bool hasCuda READ hasCuda CONSTANT)
    Q_PROPERTY(bool hasRocm READ hasRocm CONSTANT)
    Q_PROPERTY(QString recommendedBackend READ recommendedBackend CONSTANT)
    Q_PROPERTY(QString osDescription READ osDescription CONSTANT)
    Q_PROPERTY(QString desktopEnv READ desktopEnv CONSTANT)
    Q_PROPERTY(QString sessionType READ sessionType CONSTANT)

public:
    explicit HardwareInfo(QObject* parent = nullptr);

    QString cpuModel() const { return m_cpuModel; }
    int cpuThreads() const { return m_cpuThreads; }
    QString ramHuman() const { return m_ramHuman; }
    QString gpuVendor() const { return m_gpuVendor; }
    QString gpuModel() const { return m_gpuModel; }
    bool hasVulkan() const { return m_hasVulkan; }
    bool hasCuda() const { return m_hasCuda; }
    bool hasRocm() const { return m_hasRocm; }
    QString recommendedBackend() const;
    QString osDescription() const { return m_os; }
    QString desktopEnv() const { return m_de; }
    QString sessionType() const { return m_session; }

    Q_INVOKABLE QStringList availableBackends() const;

private:
    void detect();

    QString m_cpuModel;
    int m_cpuThreads = 0;
    QString m_ramHuman;
    QString m_gpuVendor;
    QString m_gpuModel;
    bool m_hasVulkan = false;
    bool m_hasCuda = false;
    bool m_hasRocm = false;
    QString m_os;
    QString m_de;
    QString m_session;
};

} // namespace dictapulse
