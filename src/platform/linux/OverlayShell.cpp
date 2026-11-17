// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "OverlayShell.h"

#include "HyprlandIpc.h"

#include <QGuiApplication>
#include <QMargins>
#include <QScreen>
#include <QWindow>

#include <cstdio>

#ifdef DICTAPULSE_HAVE_LAYERSHELL
#include <LayerShellQt/Window>
#endif

namespace dictapulse {

OverlayShell::OverlayShell(QObject* parent)
    : QObject(parent)
{
}

bool OverlayShell::wanted()
{
#ifdef DICTAPULSE_HAVE_LAYERSHELL
    if (QGuiApplication::platformName() != QLatin1String("wayland")) return false;
    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    // KWin keeps its window-rule path.
    return !desktop.contains(QStringLiteral("KDE"), Qt::CaseInsensitive);
#else
    return false;
#endif
}

#ifdef DICTAPULSE_HAVE_LAYERSHELL
namespace {
void applyAnchor(LayerShellQt::Window* lw, const QString& position)
{
    const bool top = position == QLatin1String("top-center");
    lw->setAnchors(top ? LayerShellQt::Window::AnchorTop : LayerShellQt::Window::AnchorBottom);
    // Anchored to one edge only, so the surface is centred along it.
    lw->setMargins(top ? QMargins(0, 60, 0, 0) : QMargins(0, 0, 0, 80));
}
} // namespace
#endif

bool OverlayShell::attach(QWindow* window, const QString& position)
{
#ifdef DICTAPULSE_HAVE_LAYERSHELL
    if (!wanted() || !window) return false;
    auto* lw = LayerShellQt::Window::get(window);
    if (!lw) return false;
    lw->setLayer(LayerShellQt::Window::LayerOverlay);
    lw->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    lw->setScope(QStringLiteral("dictapulse-overlay"));
    lw->setExclusiveZone(0);
    applyAnchor(lw, position);
    m_window = window;
    std::fprintf(stderr, "[DictaPulse] overlay: layer-shell surface (%s)\n", qUtf8Printable(position));
    std::fflush(stderr);
    followFocusedMonitor();
    return true;
#else
    Q_UNUSED(window)
    Q_UNUSED(position)
    return false;
#endif
}

void OverlayShell::reposition(const QString& position)
{
#ifdef DICTAPULSE_HAVE_LAYERSHELL
    if (!m_window) return;
    if (auto* lw = LayerShellQt::Window::get(m_window)) applyAnchor(lw, position);
#else
    Q_UNUSED(position)
#endif
}

void OverlayShell::followFocusedMonitor()
{
#ifdef DICTAPULSE_HAVE_LAYERSHELL
    if (!m_window || !HyprlandIpc::available()) return;
    const QString name = HyprlandIpc::focusedMonitorName();
    if (name.isEmpty()) return;
    for (QScreen* s : QGuiApplication::screens()) {
        if (s->name() != name) continue;
        if (m_window->screen() != s) {
            if (auto* lw = LayerShellQt::Window::get(m_window)) lw->setScreen(s);
            std::fprintf(stderr, "[DictaPulse] overlay: moved to focused monitor %s\n", qUtf8Printable(name));
            std::fflush(stderr);
        }
        return;
    }
#endif
}

} // namespace dictapulse
