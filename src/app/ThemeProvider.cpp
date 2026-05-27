#include "ThemeProvider.h"

#include <QEvent>
#include <QGuiApplication>
#include <QPalette>
#include <QWindow>

#ifdef DICTAPULSE_HAVE_KF6
#include <KColorScheme>
#include <KWindowEffects>
#endif

#include <cstdio>

namespace dictapulse {

ThemeProvider::ThemeProvider(QObject* parent)
    : QObject(parent)
{
    if (qApp) qApp->installEventFilter(this);
    recompute();
}

void ThemeProvider::setPreference(const QString& value)
{
    if (m_preference == value) return;
    m_preference = value;
    recompute();
}

bool ThemeProvider::eventFilter(QObject* obj, QEvent* event)
{
    // The KDE platform theme posts these when the user switches color scheme or
    // accent at runtime; re-read so the UI re-tints live.
    switch (event->type()) {
    case QEvent::ApplicationPaletteChange:
    case QEvent::ThemeChange:
    case QEvent::PaletteChange:
        recompute();
        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

void ThemeProvider::recompute()
{
    bool systemDark = true;
    QColor accent(0x7c, 0x5c, 0xff);

#ifdef DICTAPULSE_HAVE_KF6
    const KColorScheme window(QPalette::Active, KColorScheme::Window);
    systemDark = window.background().color().lightnessF() < 0.5;
    // Plasma's selection/highlight background tracks the user's accent color.
    const KColorScheme selection(QPalette::Active, KColorScheme::Selection);
    accent = selection.background(KColorScheme::NormalBackground).color();
#else
    if (qApp) {
        const QPalette pal = qApp->palette();
        systemDark = pal.color(QPalette::Window).lightnessF() < 0.5;
        accent = pal.color(QPalette::Highlight);
    }
#endif

    if (m_preference == QLatin1String("light"))
        m_dark = false;
    else if (m_preference == QLatin1String("dark"))
        m_dark = true;
    else
        m_dark = systemDark;

    m_accent = accent;
    emit changed();
}

void ThemeProvider::enableBlur(QWindow* window)
{
    if (!window) return;
#ifdef DICTAPULSE_HAVE_KF6
    KWindowEffects::enableBlurBehind(window, true);
    KWindowEffects::enableBackgroundContrast(window, true);
    m_blurEnabled = true;
    std::fprintf(stderr, "[DictaPulse] blur-behind enabled for window '%s'\n",
                 qUtf8Printable(window->objectName()));
    std::fflush(stderr);
#endif
    emit changed();
}

// --- color tokens ---------------------------------------------------------
// Alphas are 0–255. Translucent surfaces let the blur show through.

QColor ThemeProvider::bg() const
{
    // Heavier frost: ~0.88 so the desktop reads as a soft blur, not legible text.
    return m_dark ? QColor(18, 20, 27, 224)
                  : QColor(243, 245, 249, 230);
}
QColor ThemeProvider::bgRaised() const
{
    // Cards sit on top of bg; a light film lifts them into a glass panel.
    return m_dark ? QColor(255, 255, 255, 24)
                  : QColor(255, 255, 255, 205);
}
QColor ThemeProvider::bgHover() const
{
    return m_dark ? QColor(255, 255, 255, 30)
                  : QColor(0, 0, 0, 16);
}
QColor ThemeProvider::border() const
{
    return m_dark ? QColor(255, 255, 255, 30)
                  : QColor(0, 0, 0, 26);
}
QColor ThemeProvider::borderHi() const
{
    return m_dark ? QColor(255, 255, 255, 50)
                  : QColor(0, 0, 0, 46);
}
QColor ThemeProvider::text() const
{
    return m_dark ? QColor(0xf0, 0xf3, 0xf7) : QColor(0x18, 0x1b, 0x22);
}
QColor ThemeProvider::textDim() const
{
    return m_dark ? QColor(0xf0, 0xf3, 0xf7, 150) : QColor(0x18, 0x1b, 0x22, 150);
}
QColor ThemeProvider::accentSoft() const
{
    QColor c = m_accent;
    c.setAlpha(m_dark ? 52 : 40);
    return c;
}
QColor ThemeProvider::onAccent() const
{
    return m_accent.lightnessF() > 0.62 ? QColor(0x10, 0x12, 0x16)
                                        : QColor(0xff, 0xff, 0xff);
}
QColor ThemeProvider::overlayBg() const
{
    return m_dark ? QColor(10, 12, 16, 235) : QColor(244, 246, 250, 235);
}

} // namespace dictapulse
