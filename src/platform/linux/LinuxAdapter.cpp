// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "LinuxAdapter.h"

#include "ActiveWindowWatcher.h"
#include "ControlChannel.h"
#include "TextInjector.h"

#include <QAction>
#include <QDebug>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QSystemTrayIcon>

#include <cstdio>

#ifdef DICTAPULSE_HAVE_KF6
#include <KGlobalAccel>
#include <KNotification>
#include <KStatusNotifierItem>
#endif

namespace dictapulse {

namespace {
bool isKdeSession()
{
    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    return desktop.contains(QStringLiteral("KDE"), Qt::CaseInsensitive)
        || qEnvironmentVariable("KDE_FULL_SESSION") == QLatin1String("true");
}

QIcon themedIcon(const QString& name)
{
    return QIcon::fromTheme(name, QIcon(QStringLiteral(":/qt/qml/DictaPulse/icons/dictapulse.svg")));
}
} // namespace

LinuxAdapter::LinuxAdapter(QObject* parent)
    : PlatformAdapter(parent)
    , m_injector(new TextInjector(this))
    , m_activeWindow(new ActiveWindowWatcher(this))
    , m_control(new ControlChannel(this))
{
#ifdef DICTAPULSE_HAVE_KF6
    m_useKde = isKdeSession();
#endif
    std::fprintf(stderr, "[DictaPulse] desktop backend: %s\n",
                 m_useKde ? "kde" : (HyprlandIpc::available() ? "hyprland" : "generic"));
    std::fflush(stderr);

    // The control interface is available on every Linux session: it is how
    // compositors without a global-shortcut service reach the running app.
    m_control->setStatusProvider([this]() { return m_currentState; });
    connect(m_control, &ControlChannel::toggleRequested, this, &PlatformAdapter::dictationShortcutPressed);
    connect(m_control, &ControlChannel::pttPressed, this, &PlatformAdapter::pushToTalkPressed);
    connect(m_control, &ControlChannel::pttReleased, this, &PlatformAdapter::pushToTalkReleased);
    connect(m_control, &ControlChannel::cancelRequested, this, &PlatformAdapter::cancelShortcutPressed);
    connect(m_control, &ControlChannel::showRequested, this, &PlatformAdapter::traySettingsRequested);
    connect(m_control, &ControlChannel::quitRequested, this, &PlatformAdapter::trayQuitRequested);
    if (!m_control->listen()) {
        std::fprintf(stderr, "[DictaPulse] control socket unavailable (another instance?)\n");
        std::fflush(stderr);
    }

    m_animTimer.setInterval(500);
    connect(&m_animTimer, &QTimer::timeout, this, &LinuxAdapter::onAnimationTick);
    buildTrayMenu();
}

LinuxAdapter::~LinuxAdapter() = default;

QAction* LinuxAdapter::createGlobalAction(const QString& objectName,
                                          const QString& label,
                                          const QString& sequence)
{
    auto* action = new QAction(label, this);
    action->setObjectName(objectName);
#ifdef DICTAPULSE_HAVE_KF6
    KGlobalAccel::self()->setShortcut(action,
                                      { QKeySequence(sequence) },
                                      KGlobalAccel::Autoloading);
#else
    Q_UNUSED(sequence)
#endif
    return action;
}

bool LinuxAdapter::registerDictationShortcut(const QString& sequence)
{
#ifdef DICTAPULSE_HAVE_KF6
    if (m_useKde) {
        if (m_dictateAction) {
            KGlobalAccel::self()->removeAllShortcuts(m_dictateAction);
            m_dictateAction->deleteLater();
            m_dictateAction = nullptr;
        }
        m_dictateAction = createGlobalAction("dictapulse_dictate",
                                             tr("DictaPulse: Start/Stop Dictation"),
                                             sequence);
        connect(m_dictateAction, &QAction::triggered, this, &PlatformAdapter::dictationShortcutPressed);
        return true;
    }
#endif
    // No global-shortcut service: keys are bound by the compositor to the
    // control commands, so there is nothing to register here.
    Q_UNUSED(sequence)
    return m_control != nullptr;
}

bool LinuxAdapter::registerCancelShortcut(const QString& sequence)
{
#ifdef DICTAPULSE_HAVE_KF6
    if (m_useKde) {
        if (m_cancelAction) {
            KGlobalAccel::self()->removeAllShortcuts(m_cancelAction);
            m_cancelAction->deleteLater();
            m_cancelAction = nullptr;
        }
        m_cancelAction = createGlobalAction("dictapulse_cancel",
                                            tr("DictaPulse: Cancel Dictation"),
                                            sequence);
        connect(m_cancelAction, &QAction::triggered, this, &PlatformAdapter::cancelShortcutPressed);
        return true;
    }
#endif
    Q_UNUSED(sequence)
    return m_control != nullptr;
}

void LinuxAdapter::captureTarget()
{
    m_target = m_activeWindow ? m_activeWindow->activeWindow() : HyprWindow();
    std::fprintf(stderr, "[DictaPulse] captured target address=%s class='%s'\n",
                 qUtf8Printable(m_target.address), qUtf8Printable(m_target.cls));
    std::fflush(stderr);
}

PlatformAdapter::InjectResult LinuxAdapter::injectText(const QString& text,
                                                       const QString& mode,
                                                       bool clipboardFallback)
{
    InsertTarget target;
    target.window = m_target;
    target.pressEnter = m_pressEnter;
    switch (m_injector->inject(text, mode, clipboardFallback, target)) {
    case TextInjector::Result::Inserted: return InjectResult::Inserted;
    case TextInjector::Result::ClipboardOnly: return InjectResult::ClipboardOnly;
    case TextInjector::Result::FocusChanged: return InjectResult::FocusChanged;
    case TextInjector::Result::Failed: return InjectResult::Failed;
    }
    return InjectResult::Failed;
}

QString LinuxAdapter::activeWindowId() const
{
    return m_activeWindow ? m_activeWindow->activeWindowId() : QString();
}

void LinuxAdapter::showTrayMessage(const QString& title, const QString& body)
{
#ifdef DICTAPULSE_HAVE_KF6
    if (m_useKde) {
        KNotification::event(QStringLiteral("dictapulse.notify"),
                             title,
                             body,
                             QStringLiteral("audio-input-microphone"));
        return;
    }
#endif
    if (m_sysTray) {
        m_sysTray->showMessage(title, body, themedIcon(QStringLiteral("audio-input-microphone")), 6000);
    }
}

void LinuxAdapter::setTrayIconName(const QString& name)
{
#ifdef DICTAPULSE_HAVE_KF6
    if (m_tray) {
        m_tray->setIconByName(name);
        return;
    }
#endif
    if (m_sysTray) m_sysTray->setIcon(themedIcon(name));
}

void LinuxAdapter::setTrayState(const QString& state, const QString& tooltip)
{
    m_currentState = state;
#ifdef DICTAPULSE_HAVE_KF6
    if (m_tray) {
        m_tray->setToolTipSubTitle(tooltip);
        if (state == "listening") {
            m_tray->setStatus(KStatusNotifierItem::NeedsAttention);
            m_tray->setAttentionIconByName(QStringLiteral("media-record"));
            m_tray->setIconByName(QStringLiteral("audio-input-microphone"));
            if (m_animEnabled) startAnimation();
        } else if (state == "processing") {
            m_tray->setStatus(KStatusNotifierItem::Active);
            m_tray->setIconByName(QStringLiteral("system-run"));
            if (m_animEnabled) startAnimation();
            else stopAnimation();
        } else if (state == "error") {
            m_tray->setStatus(KStatusNotifierItem::NeedsAttention);
            m_tray->setAttentionIconByName(QStringLiteral("dialog-error"));
            m_tray->setIconByName(QStringLiteral("dialog-error"));
            stopAnimation();
        } else {
            m_tray->setStatus(KStatusNotifierItem::Passive);
            m_tray->setIconByName(QStringLiteral("audio-input-microphone"));
            stopAnimation();
        }
        return;
    }
#endif
    if (!m_sysTray) return;
    m_sysTray->setToolTip(QStringLiteral("DictaPulse: ") + tooltip);
    if (state == "listening") {
        setTrayIconName(QStringLiteral("media-record"));
        if (m_animEnabled) startAnimation();
    } else if (state == "processing") {
        setTrayIconName(QStringLiteral("system-run"));
        if (m_animEnabled) startAnimation();
        else stopAnimation();
    } else if (state == "error") {
        setTrayIconName(QStringLiteral("dialog-error"));
        stopAnimation();
    } else {
        setTrayIconName(QStringLiteral("audio-input-microphone"));
        stopAnimation();
    }
}

void LinuxAdapter::startAnimation()
{
    if (!m_animTimer.isActive()) {
        m_animFrame = 0;
        m_animTimer.start();
    }
}

void LinuxAdapter::stopAnimation()
{
    if (m_animTimer.isActive()) m_animTimer.stop();
}

void LinuxAdapter::onAnimationTick()
{
    m_animFrame = (m_animFrame + 1) % 2;
    if (m_currentState == "listening") {
        setTrayIconName(m_animFrame == 0 ? QStringLiteral("media-record")
                                         : QStringLiteral("audio-input-microphone"));
    } else if (m_currentState == "processing") {
        setTrayIconName(m_animFrame == 0 ? QStringLiteral("view-refresh")
                                         : QStringLiteral("system-run"));
    } else {
        stopAnimation();
    }
}

void LinuxAdapter::setAnimationEnabled(bool enabled)
{
    m_animEnabled = enabled;
    if (!enabled) {
        stopAnimation();
        // Restore a stable icon if we were mid-animation.
        if (m_currentState == "listening" || m_currentState == "processing")
            setTrayIconName(QStringLiteral("audio-input-microphone"));
    } else if (m_currentState == "listening" || m_currentState == "processing") {
        startAnimation();
    }
}

void LinuxAdapter::buildTrayMenu()
{
#ifdef DICTAPULSE_HAVE_KF6
    if (m_useKde) {
        m_tray = new KStatusNotifierItem(QStringLiteral("dictapulse"), this);
        m_tray->setTitle(QStringLiteral("DictaPulse"));
        m_tray->setIconByName(QStringLiteral("audio-input-microphone"));
        m_tray->setStatus(KStatusNotifierItem::Passive);
        m_tray->setCategory(KStatusNotifierItem::ApplicationStatus);
        m_tray->setToolTipTitle(QStringLiteral("DictaPulse"));
        m_tray->setToolTipSubTitle(tr("Ready"));
        m_tray->setStandardActionsEnabled(false);

        auto* menu = new QMenu();

        auto* dictate = menu->addAction(tr("Start / Stop Dictation"));
        connect(dictate, &QAction::triggered, this, &PlatformAdapter::dictationShortcutPressed);

        auto* cancel = menu->addAction(tr("Cancel Dictation"));
        connect(cancel, &QAction::triggered, this, &PlatformAdapter::cancelShortcutPressed);

        menu->addSeparator();
        auto* settings = menu->addAction(tr("Open Settings"));
        connect(settings, &QAction::triggered, this, &PlatformAdapter::traySettingsRequested);

        menu->addSeparator();
        auto* quit = menu->addAction(tr("Quit"));
        connect(quit, &QAction::triggered, this, &PlatformAdapter::trayQuitRequested);

        m_tray->setContextMenu(menu);
        connect(m_tray, &KStatusNotifierItem::activateRequested,
                this, [this](bool, const QPoint&) { emit traySettingsRequested(); });
        return;
    }
#endif
    buildGenericTray();
}

void LinuxAdapter::buildGenericTray()
{
    // Qt's own StatusNotifierItem implementation: no KDE libraries needed, and
    // it is what DMS, waybar, KDE and other trays host.
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        std::fprintf(stderr, "[DictaPulse] no StatusNotifier host yet; tray icon will appear when one starts\n");
        std::fflush(stderr);
    }
    m_sysTray = new QSystemTrayIcon(themedIcon(QStringLiteral("audio-input-microphone")), this);
    m_sysTray->setToolTip(QStringLiteral("DictaPulse: ") + tr("Ready"));

    m_sysMenu = new QMenu();
    auto* dictate = m_sysMenu->addAction(tr("Start / Stop Dictation"));
    connect(dictate, &QAction::triggered, this, &PlatformAdapter::dictationShortcutPressed);
    auto* cancel = m_sysMenu->addAction(tr("Cancel Dictation"));
    connect(cancel, &QAction::triggered, this, &PlatformAdapter::cancelShortcutPressed);
    m_sysMenu->addSeparator();
    auto* settings = m_sysMenu->addAction(tr("Open Settings"));
    connect(settings, &QAction::triggered, this, &PlatformAdapter::traySettingsRequested);
    m_sysMenu->addSeparator();
    auto* quit = m_sysMenu->addAction(tr("Quit"));
    connect(quit, &QAction::triggered, this, &PlatformAdapter::trayQuitRequested);

    m_sysTray->setContextMenu(m_sysMenu);
    connect(m_sysTray.data(), &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                    emit traySettingsRequested();
            });
    m_sysTray->show();
}

} // namespace dictapulse
