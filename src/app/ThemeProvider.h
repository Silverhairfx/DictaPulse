// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include <QColor>
#include <QObject>
#include <QString>

namespace dictapulse {

// Central design tokens for the QML UI, exposed to QML as the `Theme` singleton.
//
// "Tactile pop" clay design: opaque surfaces molded out
// of the canvas with a pillow-emboss treatment — inset top-left highlight,
// inset bottom-right shade, plush drop shadow. Light mode is a white canvas
// with vivid indigo primary and cool-violet neutrals; dark mode is a deep
// blue-slate scale with a luminous indigo accent.
//
// The mode follows the system (KDE color scheme) unless a manual preference
// ("light"/"dark") overrides it. The accent is fixed brand indigo per mode.
class ThemeProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString preference READ preference WRITE setPreference NOTIFY changed)
    Q_PROPERTY(QString mode READ mode NOTIFY changed)

    // Surfaces
    Q_PROPERTY(QColor bg READ bg NOTIFY changed)
    Q_PROPERTY(QColor bgRaised READ bgRaised NOTIFY changed)
    Q_PROPERTY(QColor bgHover READ bgHover NOTIFY changed)
    Q_PROPERTY(QColor bgWell READ bgWell NOTIFY changed)
    Q_PROPERTY(QColor border READ border NOTIFY changed)
    Q_PROPERTY(QColor borderHi READ borderHi NOTIFY changed)

    // Text
    Q_PROPERTY(QColor text READ text NOTIFY changed)
    Q_PROPERTY(QColor textDim READ textDim NOTIFY changed)

    // Accent (brand indigo) + status hues
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor accentSoft READ accentSoft NOTIFY changed)
    Q_PROPERTY(QColor accentSoftFg READ accentSoftFg NOTIFY changed)
    Q_PROPERTY(QColor onAccent READ onAccent NOTIFY changed)
    Q_PROPERTY(QColor success READ success NOTIFY changed)
    Q_PROPERTY(QColor warning READ warning NOTIFY changed)
    Q_PROPERTY(QColor danger READ danger NOTIFY changed)
    Q_PROPERTY(QColor info READ info NOTIFY changed)
    Q_PROPERTY(QColor overlayBg READ overlayBg NOTIFY changed)

    // Clay (pillow emboss) shadow colors
    Q_PROPERTY(QColor clayHighlight READ clayHighlight NOTIFY changed)
    Q_PROPERTY(QColor clayShade READ clayShade NOTIFY changed)
    Q_PROPERTY(QColor clayDrop READ clayDrop NOTIFY changed)
    Q_PROPERTY(QColor clayDropSoft READ clayDropSoft NOTIFY changed)

    // Atmospheric canvas washes (radial gradients behind everything)
    Q_PROPERTY(QColor washA READ washA NOTIFY changed)
    Q_PROPERTY(QColor washB READ washB NOTIFY changed)
    Q_PROPERTY(QColor washC READ washC NOTIFY changed)

    // Display (headline) font family — serif, loaded from resources at startup.
    Q_PROPERTY(QString displayFont READ displayFont NOTIFY changed)

    Q_PROPERTY(int radius READ radius CONSTANT)
    Q_PROPERTY(int radiusSm READ radiusSm CONSTANT)
    Q_PROPERTY(int pad READ pad CONSTANT)
    Q_PROPERTY(int padSm READ padSm CONSTANT)
    Q_PROPERTY(int gap READ gap CONSTANT)

public:
    explicit ThemeProvider(QObject* parent = nullptr);

    QString preference() const { return m_preference; }
    void setPreference(const QString& value);
    QString mode() const { return m_dark ? QStringLiteral("dark") : QStringLiteral("light"); }

    QColor bg() const;
    QColor bgRaised() const;
    QColor bgHover() const;
    QColor bgWell() const;
    QColor border() const;
    QColor borderHi() const;
    QColor text() const;
    QColor textDim() const;
    QColor accent() const;
    QColor accentSoft() const;
    QColor accentSoftFg() const;
    QColor onAccent() const;
    QColor success() const;
    QColor warning() const;
    QColor danger() const;
    QColor info() const;
    QColor overlayBg() const;

    QColor clayHighlight() const;
    QColor clayShade() const;
    QColor clayDrop() const;
    QColor clayDropSoft() const;

    QColor washA() const;
    QColor washB() const;
    QColor washC() const;

    QString displayFont() const { return m_displayFont; }
    void setDisplayFont(const QString& family);

    int radius() const { return 12; }
    int radiusSm() const { return 9; }
    int pad() const { return 18; }
    int padSm() const { return 11; }
    int gap() const { return 13; }

signals:
    void changed();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void recompute();

    QString m_preference = QStringLiteral("system");
    QString m_displayFont = QStringLiteral("serif");
    bool m_dark = true;
};

} // namespace dictapulse
