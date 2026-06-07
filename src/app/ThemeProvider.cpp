#include "ThemeProvider.h"

#include <QEvent>
#include <QGuiApplication>
#include <QPalette>

#ifdef DICTAPULSE_HAVE_KF6
#include <KColorScheme>
#endif

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

void ThemeProvider::setDisplayFont(const QString& family)
{
    if (m_displayFont == family) return;
    m_displayFont = family;
    emit changed();
}

bool ThemeProvider::eventFilter(QObject* obj, QEvent* event)
{
    // The KDE platform theme posts these when the user switches color scheme
    // at runtime; re-read so "system" preference follows light/dark live.
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

#ifdef DICTAPULSE_HAVE_KF6
    const KColorScheme window(QPalette::Active, KColorScheme::Window);
    systemDark = window.background().color().lightnessF() < 0.5;
#else
    if (qApp) {
        const QPalette pal = qApp->palette();
        systemDark = pal.color(QPalette::Window).lightnessF() < 0.5;
    }
#endif

    if (m_preference == QLatin1String("light"))
        m_dark = false;
    else if (m_preference == QLatin1String("dark"))
        m_dark = true;
    else
        m_dark = systemDark;

    emit changed();
}

// --- color tokens ----------------------------------------------------------
// "Tactile pop" clay palette. Light: white canvas, vivid indigo, cool
// violet neutrals. Dark: deep blue-slate scale, luminous indigo accent.
// All surfaces are opaque — depth comes from the clay emboss, not blur.

QColor ThemeProvider::bg() const
{
    return m_dark ? QColor(0x02, 0x06, 0x18)   // slate-950
                  : QColor(0xff, 0xff, 0xff);  // pure white canvas
}
QColor ThemeProvider::bgRaised() const
{
    // Cards: one step above the canvas in dark, white in light (clay shadows
    // carry the separation there).
    return m_dark ? QColor(0x0f, 0x17, 0x2b)   // slate-900
                  : QColor(0xff, 0xff, 0xff);
}
QColor ThemeProvider::bgHover() const
{
    return m_dark ? QColor(0x1d, 0x29, 0x3d)   // slate-800
                  : QColor(0xf1, 0xef, 0xff);  // soft indigo wash
}
QColor ThemeProvider::bgWell() const
{
    // Inputs read as wells molded INTO the canvas (clay-pressed).
    return m_dark ? QColor(0x18, 0x21, 0x36)
                  : QColor(0xf4, 0xf2, 0xfa);
}
QColor ThemeProvider::border() const
{
    return m_dark ? QColor(255, 255, 255, 26)
                  : QColor(0xe9, 0xe7, 0xf3);
}
QColor ThemeProvider::borderHi() const
{
    return m_dark ? QColor(255, 255, 255, 51)
                  : QColor(0xd5, 0xd1, 0xe8);
}
QColor ThemeProvider::text() const
{
    return m_dark ? QColor(0xf8, 0xfa, 0xfc)   // slate-50
                  : QColor(0x21, 0x1d, 0x35);  // deep violet-ink
}
QColor ThemeProvider::textDim() const
{
    return m_dark ? QColor(0x90, 0xa1, 0xb9)   // slate-400
                  : QColor(0x6e, 0x6a, 0x85);
}
QColor ThemeProvider::accent() const
{
    return m_dark ? QColor(0x81, 0x8c, 0xf8)   // indigo-400
                  : QColor(0x4f, 0x46, 0xe5);  // indigo-600
}
QColor ThemeProvider::accentSoft() const
{
    return m_dark ? QColor(0x81, 0x8c, 0xf8, 38)
                  : QColor(0xee, 0xf0, 0xff);
}
QColor ThemeProvider::accentSoftFg() const
{
    // Text/icon color on top of accentSoft chips.
    return m_dark ? QColor(0xa5, 0xb0, 0xfc)
                  : QColor(0x43, 0x38, 0xca);
}
QColor ThemeProvider::onAccent() const
{
    return m_dark ? QColor(0x1e, 0x1b, 0x4b)   // indigo-950 on luminous accent
                  : QColor(0xff, 0xff, 0xff);
}
QColor ThemeProvider::success() const
{
    return m_dark ? QColor(0x10, 0xb9, 0x81) : QColor(0x05, 0x96, 0x69);
}
QColor ThemeProvider::warning() const
{
    return m_dark ? QColor(0xfb, 0xbf, 0x24) : QColor(0xf5, 0x9e, 0x0b);
}
QColor ThemeProvider::danger() const
{
    return m_dark ? QColor(0xff, 0x64, 0x67) : QColor(0xef, 0x44, 0x44);
}
QColor ThemeProvider::info() const
{
    return m_dark ? QColor(0x38, 0xbd, 0xf8) : QColor(0x0e, 0xa5, 0xe9);
}
QColor ThemeProvider::overlayBg() const
{
    // The floating pill stays a solid dark capsule in both modes.
    return QColor(0x0b, 0x10, 0x20);
}

// --- clay emboss colors ----------------------------------------------------

QColor ThemeProvider::clayHighlight() const
{
    return m_dark ? QColor(255, 255, 255, 18)    // faint top light
                  : QColor(255, 255, 255, 242);  // bright pillow top
}
QColor ThemeProvider::clayShade() const
{
    return m_dark ? QColor(0, 0, 0, 115)
                  : QColor(54, 45, 87, 26);
}
QColor ThemeProvider::clayDrop() const
{
    return m_dark ? QColor(0, 0, 0, 128)
                  : QColor(54, 45, 87, 31);
}
QColor ThemeProvider::clayDropSoft() const
{
    return m_dark ? QColor(0, 0, 0, 89)
                  : QColor(54, 45, 87, 18);
}

// --- canvas washes ---------------------------------------------------------
// Three large radial washes give the flat canvas atmosphere (violet top-left,
// blue top-right, mint bottom) without breaking the opaque clay look.

QColor ThemeProvider::washA() const
{
    return m_dark ? QColor(0x16, 0x1f, 0x3a) : QColor(0xf3, 0xee, 0xfc);
}
QColor ThemeProvider::washB() const
{
    return m_dark ? QColor(0x26, 0x1b, 0x3d) : QColor(0xea, 0xf2, 0xfd);
}
QColor ThemeProvider::washC() const
{
    return m_dark ? QColor(0x10, 0x26, 0x1d) : QColor(0xe9, 0xf8, 0xf0);
}

} // namespace dictapulse
