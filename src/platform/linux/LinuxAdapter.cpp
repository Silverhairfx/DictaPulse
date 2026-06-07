// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#include "LinuxAdapter.h"

#include "ActiveWindowWatcher.h"
#include "TextInjector.h"

#include <QAction>
#include <QDebug>
#include <QKeySequence>
#include <QMenu>

#ifdef DICTAPULSE_HAVE_KF6
#include <KGlobalAccel>
#include <KNotification>
#include <KStatusNotifierItem>
#endif

namespace dictapulse {

LinuxAdapter::LinuxAdapter(QObject* parent)
    : PlatformAdapter(parent)
    , m_injector(new TextInjector(this))
    , m_activeWindow(new ActiveWindowWatcher(this))
{
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
#else
    Q_UNUSED(sequence)
    qWarning() << "KGlobalAccel unavailable; global shortcuts disabled.";
    return false;
#endif
}

bool LinuxAdapter::registerCancelShortcut(const QString& sequence)
{
#ifdef DICTAPULSE_HAVE_KF6
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
#else
    Q_UNUSED(sequence)
    return false;
#endif
}

PlatformAdapter::InjectResult LinuxAdapter::injectText(const QString& text,
                                                       const QString& mode,
                                                       bool clipboardFallback)
{
    switch (m_injector->inject(text, mode, clipboardFallback)) {
    case TextInjector::Result::Inserted: return InjectResult::Inserted;
    case TextInjector::Result::ClipboardOnly: return InjectResult::ClipboardOnly;
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
    KNotification::event(QStringLiteral("dictapulse.notify"),
                         title,
                         body,
                         QStringLiteral("audio-input-microphone"));
#else
    Q_UNUSED(title)
    Q_UNUSED(body)
#endif
}

void LinuxAdapter::setTrayState(const QString& state, const QString& tooltip)
{
    m_currentState = state;
#ifdef DICTAPULSE_HAVE_KF6
    if (!m_tray) return;
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
#else
    Q_UNUSED(tooltip)
#endif
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
#ifdef DICTAPULSE_HAVE_KF6
    if (!m_tray) return;
    m_animFrame = (m_animFrame + 1) % 2;
    if (m_currentState == "listening") {
        m_tray->setIconByName(m_animFrame == 0 ? QStringLiteral("media-record")
                                               : QStringLiteral("audio-input-microphone"));
    } else if (m_currentState == "processing") {
        m_tray->setIconByName(m_animFrame == 0 ? QStringLiteral("view-refresh")
                                               : QStringLiteral("system-run"));
    } else {
        stopAnimation();
    }
#endif
}

void LinuxAdapter::setAnimationEnabled(bool enabled)
{
    m_animEnabled = enabled;
    if (!enabled) {
        stopAnimation();
        // Restore a stable icon if we were mid-animation.
#ifdef DICTAPULSE_HAVE_KF6
        if (m_tray && (m_currentState == "listening" || m_currentState == "processing")) {
            m_tray->setIconByName(QStringLiteral("audio-input-microphone"));
        }
#endif
    } else if (m_currentState == "listening" || m_currentState == "processing") {
        startAnimation();
    }
}

void LinuxAdapter::buildTrayMenu()
{
#ifdef DICTAPULSE_HAVE_KF6
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
#endif
}

} // namespace dictapulse
