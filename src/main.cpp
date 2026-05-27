#include "app/Controller.h"
#include "app/Settings.h"
#include "app/ThemeProvider.h"
#include "core/hardware/HardwareInfo.h"
#include "core/models/ModelManager.h"
#include "platform/linux/LinuxAdapter.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QScreen>
#include <QWindow>

#include <cstdio>

#ifdef DICTAPULSE_HAVE_LAYERSHELL
#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>
#endif

namespace {
// KWin ignores client-side positioning for normal Wayland windows, so we can't
// place the frameless overlay at the bottom-center ourselves. Instead we write
// a KWin window rule (matched by the overlay's unique title) that forces the
// position + keep-above + no-border, and recompute it whenever the overlay size
// changes so it stays centered at any scale.
void writeOverlayKwinRule(int overlayW, int overlayH, const QString& position)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen || overlayW <= 0 || overlayH <= 0) return;
    const QRect geo = screen->geometry();
    const int x = geo.x() + (geo.width() - overlayW) / 2;
    // Honor the user's position setting. "cursor" can't be expressed as a static
    // KWin rule, so it falls back to bottom-center.
    const int y = (position == QLatin1String("top-center"))
                      ? geo.y() + 80
                      : geo.y() + geo.height() - overlayH - 80;

    // Stable rule id so we update (not duplicate) our own rule across runs.
    const QString uuid = QStringLiteral("5f3e8a90-7b2c-4d1e-9a6f-dcap00000001");
    const QString content = QStringLiteral(
        "[General]\n"
        "count=1\n"
        "rules=%1\n\n"
        "[%1]\n"
        "Description=DictaPulse Listening Overlay\n"
        "title=DictaPulse Listening Overlay\n"
        "titlematch=1\n"
        "position=%2,%3\n"
        "positionrule=2\n"
        "above=true\n"
        "aboverule=2\n"
        "skiptaskbar=true\n"
        "skiptaskbarrule=2\n"
        "skipswitcher=true\n"
        "skipswitcherrule=2\n"
        "noborder=true\n"
        "noborderrule=2\n").arg(uuid).arg(x).arg(y);

    QFile f(QDir::homePath() + QStringLiteral("/.config/kwinrulesrc"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    f.write(content.toUtf8());
    f.close();

    QDBusInterface kwin("org.kde.KWin", "/KWin", "org.kde.KWin",
                        QDBusConnection::sessionBus());
    if (kwin.isValid()) kwin.call("reconfigure");
    std::fprintf(stderr, "[DictaPulse] overlay kwin-rule: pos=%d,%d size=%dx%d\n",
                 x, y, overlayW, overlayH);
    std::fflush(stderr);
}
} // namespace

int main(int argc, char* argv[])
{
    // qInfo() is suppressed by default in release; we use it for the
    // [DictaPulse] diagnostic lines that drive bug reports. Force-on
    // unless the user has explicitly set QT_LOGGING_RULES themselves.
    if (qEnvironmentVariableIsEmpty("QT_LOGGING_RULES")) {
        QLoggingCategory::setFilterRules(QStringLiteral("*.info=true"));
    }

    // Note: LayerShellQt::Shell::useLayerShell() is deprecated since Qt 6.5 —
    // layer-shell-qt 6.6+ auto-initializes via the Wayland QPA plugin. We just
    // need to call LayerShellQt::Window::get(window) on the overlay before its
    // first show and KWin will map it as a layer-shell surface.
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("DictaPulse");
    QCoreApplication::setOrganizationDomain("dictapulse.app");
    QCoreApplication::setApplicationName("DictaPulse");
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setDesktopFileName(QStringLiteral("dictapulse"));

    if (QIcon::fromTheme("audio-input-microphone").isNull()) {
        QIcon::setThemeName("breeze");
    }

    // Material controls (flat, rounded, modern) themed to our glass palette in
    // Main.qml — replaces Fusion, whose 3D look read as dated.
    QQuickStyle::setStyle(QStringLiteral("Material"));

    auto* settings = new dictapulse::Settings(&app);

    // Theme singleton: adapts to the KDE color scheme + accent, honors the
    // user's light/dark preference, and drives the frosted-glass blur. Exposed
    // to QML as `Theme` (replaces the old static Theme.qml).
    auto* theme = new dictapulse::ThemeProvider(&app);
    theme->setPreference(settings->theme());
    QObject::connect(settings, &dictapulse::Settings::themeChanged, theme,
                     [theme, settings]() { theme->setPreference(settings->theme()); });
    qmlRegisterSingletonInstance("DictaPulse", 1, 0, "Theme", theme);

    auto* hardware = new dictapulse::HardwareInfo(&app);
    auto* models = new dictapulse::ModelManager(&app);
    auto* platform = new dictapulse::LinuxAdapter(&app);
    platform->setAnimationEnabled(settings->trayIconAnimation());
    QObject::connect(settings, &dictapulse::Settings::trayIconAnimationChanged,
                     platform, [platform, settings]() {
                         platform->setAnimationEnabled(settings->trayIconAnimation());
                     });
    auto* controller = new dictapulse::Controller(settings, models, hardware, platform, &app);

    // Wire notifications: the controller emits notify(); route it to the tray
    // notification, gated by the user's "notifications" toggle. (Previously the
    // notify signal had no consumer, so notifications never appeared and the
    // toggle did nothing.)
    QObject::connect(controller, &dictapulse::Controller::notify, platform,
                     [platform, settings](const QString& title, const QString& body) {
                         if (settings->notificationsEnabled())
                             platform->showTrayMessage(title, body);
                     });

    // "Launch at startup" — materialize/remove an XDG autostart entry so the
    // toggle actually takes effect. (Previously stored but never acted on.)
    auto applyAutostart = [settings]() {
        const QString dir = QDir::homePath() + QStringLiteral("/.config/autostart");
        const QString path = dir + QStringLiteral("/dictapulse.desktop");
        if (settings->launchAtStartup()) {
            QDir().mkpath(dir);
            QFile f(path);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                f.write(QStringLiteral(
                    "[Desktop Entry]\n"
                    "Type=Application\n"
                    "Name=DictaPulse\n"
                    "Exec=%1\n"
                    "Icon=dictapulse\n"
                    "X-GNOME-Autostart-enabled=true\n"
                    "Comment=Voice-to-text dictation\n")
                    .arg(QCoreApplication::applicationFilePath()).toUtf8());
            }
        } else {
            QFile::remove(path);
        }
    };
    applyAutostart();
    QObject::connect(settings, &dictapulse::Settings::launchAtStartupChanged, &app, applyAutostart);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", controller);
    engine.rootContext()->setContextProperty("appSettings", settings);
    engine.rootContext()->setContextProperty("modelManager", models);
    engine.rootContext()->setContextProperty("hardwareInfo", hardware);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("DictaPulse", "Main");

    // Layer-shell (the "proper" Wayland overlay protocol) was tried but KWin
    // never rendered the QQuickWindow content through it. The plain frameless
    // window DOES render; we just can't position it from the client, so a KWin
    // window rule (writeOverlayKwinRule) forces bottom-center + keep-above.
    // Find the overlay window, seed the rule from its current size, and keep the
    // rule in sync when the size slider changes.
    // Frost the desktop behind the main settings window. The window starts
    // hidden when startMinimized is set, so (re)apply blur whenever it becomes
    // visible — calling before the window has a platform surface is a no-op.
    for (QWindow* w : QGuiApplication::topLevelWindows()) {
        if (w->objectName() != QLatin1String("mainWin")) continue;
        if (w->isVisible()) theme->enableBlur(w);
        QObject::connect(w, &QWindow::visibleChanged, w, [theme, w](bool vis) {
            if (vis) theme->enableBlur(w);
        });
        break;
    }

    for (QWindow* w : QGuiApplication::topLevelWindows()) {
        if (w->objectName() != QLatin1String("overlayWin")) continue;
        writeOverlayKwinRule(w->width(), w->height(), settings->overlayPosition());
        QObject::connect(w, &QWindow::widthChanged, w, [w, settings]() {
            writeOverlayKwinRule(w->width(), w->height(), settings->overlayPosition());
        });
        QObject::connect(w, &QWindow::heightChanged, w, [w, settings]() {
            writeOverlayKwinRule(w->width(), w->height(), settings->overlayPosition());
        });
        QObject::connect(settings, &dictapulse::Settings::overlayPositionChanged, w, [w, settings]() {
            writeOverlayKwinRule(w->width(), w->height(), settings->overlayPosition());
        });
        break;
    }

    return app.exec();
}
