// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "HardwareInfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSysInfo>
#include <QTextStream>
#include <QThread>

namespace dictapulse {

namespace {
QString readCpuModel()
{
    QFile f("/proc/cpuinfo");
    if (!f.open(QIODevice::ReadOnly)) return {};
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        const QString line = ts.readLine();
        if (line.startsWith("model name")) {
            const int colon = line.indexOf(':');
            if (colon > 0) return line.mid(colon + 1).trimmed();
        }
    }
    return {};
}

qint64 readMemTotalKB()
{
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly)) return 0;
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        const QString line = ts.readLine();
        if (line.startsWith("MemTotal:")) {
            const QStringList parts = line.split(QRegularExpression(R"(\s+)"), Qt::SkipEmptyParts);
            if (parts.size() >= 2) return parts[1].toLongLong();
        }
    }
    return 0;
}

bool fileExists(const QString& p) { return QFileInfo::exists(p); }
} // namespace

HardwareInfo::HardwareInfo(QObject* parent)
    : QObject(parent)
{
    detect();
}

void HardwareInfo::detect()
{
    m_cpuModel = readCpuModel();
    if (m_cpuModel.isEmpty()) m_cpuModel = QStringLiteral("Unknown CPU");
    m_cpuThreads = QThread::idealThreadCount();

    const qint64 kb = readMemTotalKB();
    m_ramHuman = kb > 0 ? QLocale().formattedDataSize(kb * 1024) : QStringLiteral("Unknown");

    m_os = QString("%1 %2").arg(QSysInfo::prettyProductName(), QSysInfo::kernelVersion());

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_de = env.value("XDG_CURRENT_DESKTOP", env.value("DESKTOP_SESSION", "Unknown"));
    m_session = env.value("XDG_SESSION_TYPE", "unknown");

    m_hasVulkan = fileExists("/usr/lib/libvulkan.so.1")
        || fileExists("/usr/lib64/libvulkan.so.1")
        || fileExists("/usr/lib/x86_64-linux-gnu/libvulkan.so.1");

    m_hasCuda = fileExists("/usr/lib/libcuda.so")
        || fileExists("/usr/lib64/libcuda.so")
        || fileExists("/usr/lib/x86_64-linux-gnu/libcuda.so")
        || fileExists("/proc/driver/nvidia/version");

    m_hasRocm = fileExists("/opt/rocm")
        || fileExists("/usr/lib/libamdhip64.so")
        || fileExists("/usr/lib64/libamdhip64.so");

    // GPU detection via /sys/class/drm/card*/device/{vendor,uevent}
    QDir drm("/sys/class/drm");
    const QStringList cards = drm.entryList({ "card*" }, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& card : cards) {
        if (card.contains('-')) continue; // skip card0-HDMI etc
        QFile vendorFile(drm.absoluteFilePath(card + "/device/vendor"));
        QFile ueventFile(drm.absoluteFilePath(card + "/device/uevent"));
        QString vendorId;
        if (vendorFile.open(QIODevice::ReadOnly)) {
            vendorId = QString::fromUtf8(vendorFile.readAll()).trimmed();
        }
        if (vendorId == "0x10de") m_gpuVendor = "NVIDIA";
        else if (vendorId == "0x1002") m_gpuVendor = "AMD";
        else if (vendorId == "0x8086") m_gpuVendor = "Intel";

        if (ueventFile.open(QIODevice::ReadOnly)) {
            const QString u = QString::fromUtf8(ueventFile.readAll());
            for (const QString& line : u.split('\n')) {
                if (line.startsWith("DRIVER=")) {
                    const QString drv = line.mid(7);
                    if (drv == "nvidia") m_gpuVendor = "NVIDIA";
                    else if (drv == "amdgpu") m_gpuVendor = "AMD";
                    else if (drv == "i915" || drv == "xe") m_gpuVendor = "Intel";
                    if (m_gpuModel.isEmpty()) m_gpuModel = drv;
                }
            }
        }
        if (!m_gpuVendor.isEmpty()) break;
    }
    if (m_gpuVendor.isEmpty()) m_gpuVendor = "Unknown";
    if (m_gpuModel.isEmpty()) m_gpuModel = "Unknown";
}

QString HardwareInfo::recommendedBackend() const
{
    if (m_hasCuda && m_gpuVendor == "NVIDIA") return "cuda";
    if (m_hasRocm && m_gpuVendor == "AMD") return "hip";
    if (m_hasVulkan && m_gpuVendor != "Unknown") return "vulkan";
    return "cpu";
}

QStringList HardwareInfo::availableBackends() const
{
    QStringList list { "cpu" };
    if (m_hasVulkan) list << "vulkan";
    if (m_hasCuda) list << "cuda";
    if (m_hasRocm) list << "hip";
    return list;
}

} // namespace dictapulse
