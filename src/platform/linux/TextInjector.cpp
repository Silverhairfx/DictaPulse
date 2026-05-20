#include "TextInjector.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QThread>

namespace dictapulse {

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
    QClipboard* cb = QGuiApplication::clipboard();
    auto* mime = new QMimeData;
    mime->setText(text);
    cb->setMimeData(mime, QClipboard::Clipboard);
}

bool TextInjector::typeViaWtype(const QString& text)
{
    QProcess p;
    p.start("wtype", { "-s", "1", "--", text });
    if (!p.waitForStarted(1500)) return false;
    if (!p.waitForFinished(15000)) {
        p.kill();
        return false;
    }
    return p.exitCode() == 0;
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
    QThread::msleep(50);
    if (m_isWayland && m_hasWtype) {
        QProcess p;
        p.start("wtype", { "-M", "ctrl", "v", "-m", "ctrl" });
        if (p.waitForStarted(1500) && p.waitForFinished(5000)) return p.exitCode() == 0;
    }
    if (m_hasXdotool) {
        QProcess p;
        p.start("xdotool", { "key", "--clearmodifiers", "ctrl+v" });
        if (p.waitForStarted(1500) && p.waitForFinished(5000)) return p.exitCode() == 0;
    }
    if (m_hasYdotool) {
        QProcess p;
        p.start("ydotool", { "key", "29:1", "47:1", "47:0", "29:0" });
        if (p.waitForStarted(1500) && p.waitForFinished(5000)) return p.exitCode() == 0;
    }
    return false;
}

TextInjector::Result TextInjector::inject(const QString& text, const QString& mode, bool clipboardFallback)
{
    if (text.isEmpty()) return Result::Failed;

    if (mode == "clipboard") {
        copyToClipboard(text);
        return Result::ClipboardOnly;
    }

    if (mode == "copy-paste") {
        copyToClipboard(text);
        if (sendPasteShortcut()) return Result::Inserted;
        return Result::ClipboardOnly;
    }

    // mode == "insert" or unknown → direct typing
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
