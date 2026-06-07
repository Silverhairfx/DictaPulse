#include "ActiveWindowWatcher.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include <cstdio>

namespace dictapulse {

namespace {
constexpr auto kService = "org.dictapulse.helper";
constexpr auto kPath = "/ActiveWindow";

// KWin script: report the focused window's class to us on every activation.
// Plasma 6 renamed activeClient→activeWindow / clientActivated→windowActivated;
// fall back to the old names so this works across 6.x.
constexpr auto kKwinScript = R"JS(
function dpReport() {
    var w = (typeof workspace.activeWindow !== 'undefined')
        ? workspace.activeWindow : workspace.activeClient;
    var cls = w ? (w.resourceClass || w.resourceName || "") : "";
    callDBus("org.dictapulse.helper", "/ActiveWindow",
             "org.dictapulse.ActiveWindow", "report", "" + cls);
}
if (typeof workspace.windowActivated !== 'undefined')
    workspace.windowActivated.connect(dpReport);
else if (typeof workspace.clientActivated !== 'undefined')
    workspace.clientActivated.connect(dpReport);
dpReport();
)JS";
} // namespace

ActiveWindowWatcher::ActiveWindowWatcher(QObject* parent)
    : QObject(parent)
{
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_isWayland = env.value("XDG_SESSION_TYPE").compare("wayland", Qt::CaseInsensitive) == 0
        || !env.value("WAYLAND_DISPLAY").isEmpty();

    if (m_isWayland) setupWayland();
}

ActiveWindowWatcher::~ActiveWindowWatcher()
{
    if (m_scriptId >= 0) {
        QDBusInterface kwin("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting",
                            QDBusConnection::sessionBus());
        if (kwin.isValid()) kwin.call("unloadScript", QStringLiteral("dictapulse-activewindow"));
    }
}

void ActiveWindowWatcher::report(const QString& windowClass)
{
    m_current = windowClass.trimmed().toLower();
}

QString ActiveWindowWatcher::activeWindowId() const
{
    return m_isWayland ? m_current : queryX11();
}

void ActiveWindowWatcher::setupWayland()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) return;

    // Host the callback object the KWin script will push activations to.
    bus.registerService(QString::fromLatin1(kService));
    if (!bus.registerObject(QString::fromLatin1(kPath), this,
                            QDBusConnection::ExportScriptableSlots)) {
        std::fprintf(stderr, "[DictaPulse] active-window: failed to register D-Bus object\n");
        std::fflush(stderr);
        return;
    }

    // Materialize the KWin script on disk — loadScript needs a real path.
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    const QString scriptPath = (dir.isEmpty() ? QDir::tempPath() : dir)
        + QStringLiteral("/dictapulse-activewindow.js");
    QFile f(scriptPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        std::fprintf(stderr, "[DictaPulse] active-window: cannot write KWin script\n");
        std::fflush(stderr);
        return;
    }
    f.write(QByteArray(kKwinScript));
    f.close();

    QDBusInterface kwin("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting", bus);
    if (!kwin.isValid()) {
        std::fprintf(stderr, "[DictaPulse] active-window: KWin scripting unavailable\n");
        std::fflush(stderr);
        return;
    }
    // Unload any stale copy from a previous run, then load + run ours.
    kwin.call("unloadScript", QStringLiteral("dictapulse-activewindow"));
    QDBusReply<int> id = kwin.call("loadScript", scriptPath,
                                   QStringLiteral("dictapulse-activewindow"));
    if (!id.isValid() || id.value() < 0) {
        std::fprintf(stderr, "[DictaPulse] active-window: loadScript failed\n");
        std::fflush(stderr);
        return;
    }
    m_scriptId = id.value();
    QDBusInterface script("org.kde.KWin",
                          QStringLiteral("/Scripting/Script%1").arg(m_scriptId),
                          "org.kde.kwin.Script", bus);
    script.call("run");
    std::fprintf(stderr, "[DictaPulse] active-window: KWin watcher loaded (id=%d)\n", m_scriptId);
    std::fflush(stderr);
}

QString ActiveWindowWatcher::queryX11() const
{
    if (QStandardPaths::findExecutable("xdotool").isEmpty()) return {};
    QProcess p;
    p.start("xdotool", { "getactivewindow", "getwindowclassname" });
    if (!p.waitForStarted(800)) return {};
    if (!p.waitForFinished(800)) { p.kill(); return {}; }
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed().toLower();
}

} // namespace dictapulse
