// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Tymour Kadry / ETK Technologies <https://etk-tech.com>
#pragma once

#include "platform/PlatformAdapter.h"

#include <QPointer>
#include <QTimer>

class QAction;

#ifdef DICTAPULSE_HAVE_KF6
class KStatusNotifierItem;
#endif

namespace dictapulse {

class TextInjector;
class ActiveWindowWatcher;

class LinuxAdapter : public PlatformAdapter {
    Q_OBJECT
public:
    explicit LinuxAdapter(QObject* parent = nullptr);
    ~LinuxAdapter() override;

    bool registerDictationShortcut(const QString& sequence) override;
    bool registerCancelShortcut(const QString& sequence) override;

    InjectResult injectText(const QString& text,
                            const QString& mode,
                            bool clipboardFallback) override;

    void showTrayMessage(const QString& title, const QString& body) override;
    void setTrayState(const QString& state, const QString& tooltip) override;
    QString activeWindowId() const override;

    void setAnimationEnabled(bool enabled);

private:
    QAction* createGlobalAction(const QString& objectName,
                                const QString& label,
                                const QString& sequence);
    void buildTrayMenu();
    void startAnimation();
    void stopAnimation();
    void onAnimationTick();

    QAction* m_dictateAction = nullptr;
    QAction* m_cancelAction = nullptr;
    TextInjector* m_injector = nullptr;
    ActiveWindowWatcher* m_activeWindow = nullptr;
    QTimer m_animTimer;
    QString m_currentState = "idle";
    int m_animFrame = 0;
    bool m_animEnabled = true;
#ifdef DICTAPULSE_HAVE_KF6
    KStatusNotifierItem* m_tray = nullptr;
#endif
};

} // namespace dictapulse
