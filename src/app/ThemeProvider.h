#pragma once

#include <QColor>
#include <QObject>
#include <QString>

class QWindow;

namespace dictapulse {

// Central design tokens for the QML UI, exposed to QML as the `Theme` singleton.
// Colors adapt to the active KDE color scheme (light/dark) and the system accent
// color; a manual preference ("system"/"light"/"dark") can override the mode.
// Also drives the frosted-glass window blur via KWindowEffects.
//
// Surfaces are intentionally translucent so the KWin blur behind the window
// shows through (the "glass" look). If blur is unavailable the alphas still
// read as a tinted panel, just without the live frost.
class ThemeProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString preference READ preference WRITE setPreference NOTIFY changed)
    Q_PROPERTY(QString mode READ mode NOTIFY changed)
    Q_PROPERTY(bool blurEnabled READ blurEnabled NOTIFY changed)

    Q_PROPERTY(QColor bg READ bg NOTIFY changed)
    Q_PROPERTY(QColor bgRaised READ bgRaised NOTIFY changed)
    Q_PROPERTY(QColor bgHover READ bgHover NOTIFY changed)
    Q_PROPERTY(QColor border READ border NOTIFY changed)
    Q_PROPERTY(QColor borderHi READ borderHi NOTIFY changed)
    Q_PROPERTY(QColor text READ text NOTIFY changed)
    Q_PROPERTY(QColor textDim READ textDim NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor accentSoft READ accentSoft NOTIFY changed)
    Q_PROPERTY(QColor onAccent READ onAccent NOTIFY changed)
    Q_PROPERTY(QColor success READ success NOTIFY changed)
    Q_PROPERTY(QColor warning READ warning NOTIFY changed)
    Q_PROPERTY(QColor danger READ danger NOTIFY changed)
    Q_PROPERTY(QColor overlayBg READ overlayBg NOTIFY changed)

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
    bool blurEnabled() const { return m_blurEnabled; }

    QColor bg() const;
    QColor bgRaised() const;
    QColor bgHover() const;
    QColor border() const;
    QColor borderHi() const;
    QColor text() const;
    QColor textDim() const;
    QColor accent() const { return m_accent; }
    QColor accentSoft() const;
    QColor onAccent() const;
    QColor success() const { return QColor(0x2e, 0xcc, 0x71); }
    QColor warning() const { return QColor(0xf5, 0xa6, 0x23); }
    QColor danger() const { return QColor(0xff, 0x5a, 0x5a); }
    QColor overlayBg() const;

    int radius() const { return 16; }
    int radiusSm() const { return 10; }
    int pad() const { return 18; }
    int padSm() const { return 11; }
    int gap() const { return 13; }

    // Request the compositor to frost the desktop behind this window. No-op
    // without KF6/KWin; safe to call once the window has a platform surface.
    Q_INVOKABLE void enableBlur(QWindow* window);

signals:
    void changed();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void recompute();

    QString m_preference = QStringLiteral("system");
    bool m_dark = true;
    bool m_blurEnabled = false;
    QColor m_accent = QColor(0x7c, 0x5c, 0xff);
};

} // namespace dictapulse
