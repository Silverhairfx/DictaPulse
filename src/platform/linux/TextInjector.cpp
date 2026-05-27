#include "TextInjector.h"

#include <QClipboard>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QThread>

#include <cstdio>

namespace dictapulse {

namespace {
// KDE Plasma keyboard-layout DBus endpoint.
constexpr auto kKbdService = "org.kde.keyboard";
constexpr auto kKbdPath = "/Layouts";
constexpr auto kKbdIface = "org.kde.KeyboardLayouts";

// Layout codes that map keycode 47 → keysym "V" (i.e. paste shortcuts work).
// Non-Latin layouts (ar/eg/ru/fa/he…) remap it, breaking Ctrl+Shift+V.
bool isLatinLayout(const QString& code)
{
    static const QStringList latin = {
        "us", "gb", "de", "fr", "es", "it", "pt", "nl", "se", "no", "dk",
        "fi", "pl", "cz", "tr", "ca", "latam", "br", "ch", "be", "ie"
    };
    return latin.contains(code);
}
} // namespace

TextInjector::TextInjector(QObject* parent)
    : QObject(parent)
{
    m_hasWtype = !QStandardPaths::findExecutable("wtype").isEmpty();
    m_hasXdotool = !QStandardPaths::findExecutable("xdotool").isEmpty();
    m_hasYdotool = !QStandardPaths::findExecutable("ydotool").isEmpty();

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_isWayland = env.value("XDG_SESSION_TYPE").compare("wayland", Qt::CaseInsensitive) == 0
        || !env.value("WAYLAND_DISPLAY").isEmpty();
}

void TextInjector::copyToClipboard(const QString& text)
{
    // Set Qt's clipboard for in-app use and X11 fallback.
    QClipboard* cb = QGuiApplication::clipboard();
    auto* mime = new QMimeData;
    mime->setText(text);
    cb->setMimeData(mime, QClipboard::Clipboard);

    // Prefer Klipper's own setter when it's running. Klipper is KDE's clipboard
    // manager and will otherwise *revert* the clipboard to its last history item
    // when an external client (wl-copy / Qt) grabs ownership — which produced
    // stale pastes. Setting through Klipper makes Klipper the authoritative
    // owner, so the content sticks.
    bool klipperOk = false;
    {
        QDBusInterface klipper("org.kde.klipper", "/klipper", "org.kde.klipper.klipper",
                               QDBusConnection::sessionBus());
        if (klipper.isValid()) {
            QDBusReply<void> reply = klipper.call("setClipboardContents", text);
            klipperOk = reply.isValid();
            std::fprintf(stderr, "[DictaPulse] klipper.setClipboardContents ok=%d\n", klipperOk);
            std::fflush(stderr);
        }
    }

    // Fall back to wl-copy only when Klipper isn't available (e.g. Klipper
    // disabled). Its forked helper holds the wl_data_source independently.
    if (!klipperOk && m_isWayland && !QStandardPaths::findExecutable("wl-copy").isEmpty()) {
        QProcess* p = new QProcess;
        p->setProcessChannelMode(QProcess::MergedChannels);
        p->start("wl-copy", { "-n", "--type", "text/plain;charset=utf-8" });
        if (p->waitForStarted(1500)) {
            p->write(text.toUtf8());
            p->closeWriteChannel();
            QObject::connect(p, &QProcess::finished, p, &QObject::deleteLater);
            std::fprintf(stderr, "[DictaPulse] wl-copy(clipboard): %lld bytes handed off\n",
                         static_cast<long long>(text.toUtf8().size()));
            std::fflush(stderr);
        } else {
            p->deleteLater();
        }
    }
}

int TextInjector::currentKbLayout()
{
    QDBusInterface kbd(kKbdService, kKbdPath, kKbdIface, QDBusConnection::sessionBus());
    if (!kbd.isValid()) return -1;
    QDBusReply<uint> reply = kbd.call("getLayout");
    return reply.isValid() ? static_cast<int>(reply.value()) : -1;
}

int TextInjector::findLatinLayout()
{
    QDBusInterface kbd(kKbdService, kKbdPath, kKbdIface, QDBusConnection::sessionBus());
    if (!kbd.isValid()) return -1;
    QDBusMessage msg = kbd.call("getLayoutsList");
    if (msg.type() != QDBusMessage::ReplyMessage || msg.arguments().isEmpty()) return -1;
    const QDBusArgument arg = msg.arguments().first().value<QDBusArgument>();
    int idx = 0;
    int found = -1;
    arg.beginArray();
    while (!arg.atEnd()) {
        arg.beginStructure();
        QString code, variant, display;
        arg >> code >> variant >> display;
        arg.endStructure();
        if (found < 0 && isLatinLayout(code)) found = idx;
        ++idx;
    }
    arg.endArray();
    return found;
}

bool TextInjector::setKbLayout(int index)
{
    if (index < 0) return false;
    QDBusInterface kbd(kKbdService, kKbdPath, kKbdIface, QDBusConnection::sessionBus());
    if (!kbd.isValid()) return false;
    QDBusReply<bool> reply = kbd.call("setLayout", static_cast<uint>(index));
    return reply.isValid() && reply.value();
}

bool TextInjector::typeViaWtype(const QString& text)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start("wtype", { "-s", "1", "--", text });
    if (!p.waitForStarted(1500)) {
        std::fprintf(stderr, "[DictaPulse] wtype failed to start\n");
        std::fflush(stderr);
        return false;
    }
    if (!p.waitForFinished(15000)) {
        p.kill();
        std::fprintf(stderr, "[DictaPulse] wtype timed out\n");
        std::fflush(stderr);
        return false;
    }
    const int rc = p.exitCode();
    const QByteArray out = p.readAll();
    std::fprintf(stderr, "[DictaPulse] wtype exit=%d output='%s'\n",
                 rc, out.constData());
    std::fflush(stderr);
    return rc == 0;
}

bool TextInjector::typeViaXdotool(const QString& text)
{
    QProcess p;
    p.start("xdotool", { "type", "--delay", "1", "--", text });
    if (!p.waitForStarted(1500)) return false;
    if (!p.waitForFinished(15000)) {
        p.kill();
        return false;
    }
    return p.exitCode() == 0;
}

bool TextInjector::typeViaYdotool(const QString& text)
{
    QProcess p;
    p.start("ydotool", { "type", "--", text });
    if (!p.waitForStarted(1500)) return false;
    if (!p.waitForFinished(15000)) {
        p.kill();
        return false;
    }
    return p.exitCode() == 0;
}

bool TextInjector::sendPasteShortcut()
{
    // Give wl-copy's detached helper time to claim the wl_data_source so the
    // compositor advertises our text to the focused window before the paste.
    QThread::msleep(150);

    // ydotool sends raw scancodes which KWin maps through the *active* keyboard
    // layout. On a non-Latin layout (Arabic/Cyrillic/…) keycode 47 is not "V",
    // so Ctrl+Shift+V never matches the app's paste binding. Temporarily switch
    // to a Latin layout for the keystroke, then restore the user's layout.
    const int originalLayout = currentKbLayout();
    const int latinLayout = findLatinLayout();
    bool switched = false;
    if (originalLayout >= 0 && latinLayout >= 0 && originalLayout != latinLayout) {
        switched = setKbLayout(latinLayout);
        if (switched) {
            std::fprintf(stderr, "[DictaPulse] kb-layout: %d → %d (Latin) for paste\n",
                         originalLayout, latinLayout);
            std::fflush(stderr);
            QThread::msleep(60); // let the layout change settle before keystroke
        }
    }

    auto tryRun = [](const QString& bin, const QStringList& args) {
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        p.start(bin, args);
        if (!p.waitForStarted(1500)) return false;
        if (!p.waitForFinished(5000)) { p.kill(); return false; }
        const int rc = p.exitCode();
        if (rc != 0) {
            std::fprintf(stderr, "[DictaPulse] %s paste failed exit=%d output='%s'\n",
                         qUtf8Printable(bin), rc, p.readAll().constData());
            std::fflush(stderr);
        }
        return rc == 0;
    };

    // Ctrl+Shift+V works in terminals (Konsole/alacritty/kitty/foot) and as
    // paste-without-formatting in most GUI apps. With the layout now Latin,
    // keycode 47 reliably produces keysym "V".
    bool ok = false;
    if (m_isWayland && m_hasWtype && tryRun("wtype", {"-M", "ctrl", "-M", "shift", "v", "-m", "shift", "-m", "ctrl"})) ok = true;
    if (!ok && m_hasXdotool && tryRun("xdotool", {"key", "--clearmodifiers", "ctrl+shift+v"})) ok = true;
    // ydotool keycodes: 29=LeftCtrl, 42=LeftShift, 47=V.
    if (!ok && m_hasYdotool && tryRun("ydotool", {"key", "29:1", "42:1", "47:1", "47:0", "42:0", "29:0"})) ok = true;

    if (switched) {
        QThread::msleep(40);
        setKbLayout(originalLayout);
        std::fprintf(stderr, "[DictaPulse] kb-layout: restored to %d\n", originalLayout);
        std::fflush(stderr);
    }
    return ok;
}

bool TextInjector::needsPasteRoute(const QString& text)
{
    // ydotool/wtype simulate keyboard scancodes. The compositor then translates
    // those scancodes through the *current* XKB keyboard layout, so:
    //   - non-ASCII text can't be typed via scancodes at all
    //   - even ASCII text gets mangled when the user's layout is non-US
    //     (e.g. on Arabic layout, scancode for 'h' produces 'أ')
    // The only layout-safe injection path is clipboard + Shift+Insert, so
    // we always take that route on Wayland.
    Q_UNUSED(text)
    return true;
}

TextInjector::Result TextInjector::inject(const QString& text, const QString& mode, bool clipboardFallback)
{
    if (text.isEmpty()) return Result::Failed;

    if (mode == "clipboard") {
        copyToClipboard(text);
        return Result::ClipboardOnly;
    }

    const bool forcePaste = needsPasteRoute(text);

    if (mode == "copy-paste" || forcePaste) {
        copyToClipboard(text);
        std::fprintf(stderr, "[DictaPulse] inject route=copy-paste (forcePaste=%d)\n", forcePaste);
        std::fflush(stderr);
        if (sendPasteShortcut()) return Result::Inserted;
        return Result::ClipboardOnly;
    }

    // mode == "insert" or unknown → direct typing of ASCII
    bool typed = false;
    if (m_isWayland && m_hasWtype) typed = typeViaWtype(text);
    if (!typed && m_hasXdotool && !m_isWayland) typed = typeViaXdotool(text);
    if (!typed && m_hasYdotool) typed = typeViaYdotool(text);
    if (typed) return Result::Inserted;

    if (clipboardFallback) {
        copyToClipboard(text);
        if (sendPasteShortcut()) return Result::Inserted;
        return Result::ClipboardOnly;
    }
    return Result::Failed;
}

} // namespace dictapulse
