// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "InputDeviceSelector.h"

#include <QtTest>

using namespace dictapulse;

namespace {
QVector<InputCandidate> sample()
{
    return {
        { "alsa_input.pci-0000_0e_00.6.analog-stereo", "Ryzen HD Audio Controller Analog Stereo", true },
        { "alsa_input.usb-KTMicro_BY-CM3_2020-05-10-0000-0000-0000--00.analog-stereo", "BY-CM3 Analog Stereo", false },
        { "alsa_input.usb-Sonix_Technology_rapoo_camera.analog-stereo", "rapoo camera Analog Stereo", false },
        { "alsa_output.pci-0000_0e_00.6.iec958-stereo.monitor", "Monitor of Ryzen HD Audio Controller", false },
    };
}
} // namespace

class TstInputDevice : public QObject {
    Q_OBJECT
private slots:
    void patternsParse()
    {
        QCOMPARE(parsePreferredPatterns(" BOYA , by-cm3,,"), (QStringList { "boya", "by-cm3" }));
        QVERIFY(parsePreferredPatterns("").isEmpty());
    }
    void preferredMicWinsOverDefault()
    {
        QCOMPARE(pickInputDevice(sample(), QString(), parsePreferredPatterns("boya,by-cm3")), 1);
    }
    void noPatternUsesSystemDefault()
    {
        QCOMPARE(pickInputDevice(sample(), QString(), {}), 0);
    }
    void patternWithoutMatchUsesDefault()
    {
        QCOMPARE(pickInputDevice(sample(), QString(), parsePreferredPatterns("boya")), 0);
    }
    void explicitIdBeatsPatterns()
    {
        QCOMPARE(pickInputDevice(sample(), QStringLiteral("alsa_input.usb-Sonix_Technology_rapoo_camera.analog-stereo"),
                                 parsePreferredPatterns("by-cm3")), 2);
    }
    void missingExplicitIdFallsThrough()
    {
        QCOMPARE(pickInputDevice(sample(), QStringLiteral("gone"), parsePreferredPatterns("by-cm3")), 1);
    }
    void monitorsAreNeverPreferred()
    {
        QCOMPARE(pickInputDevice(sample(), QString(), parsePreferredPatterns("ryzen hd audio controller")), 0);
        QVector<InputCandidate> onlyMonitor = { sample().at(3) };
        // A monitor is skipped by pattern matching; with no default it falls back to index 0.
        QCOMPARE(pickInputDevice(onlyMonitor, QString(), parsePreferredPatterns("ryzen")), 0);
    }
    void emptyList()
    {
        QCOMPARE(pickInputDevice({}, QString(), parsePreferredPatterns("boya")), -1);
    }
};

QTEST_GUILESS_MAIN(TstInputDevice)
#include "tst_input_device.moc"
