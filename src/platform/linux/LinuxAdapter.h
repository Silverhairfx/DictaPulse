#pragma once

#include "platform/PlatformAdapter.h"

#include <QPointer>

class QAction;

#ifdef DICTAPULSE_HAVE_KF6
class KStatusNotifierItem;
#endif

namespace dictapulse {

class TextInjector;

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

private:
    QAction* createGlobalAction(const QString& objectName,
                                const QString& label,
                                const QString& sequence);
    void buildTrayMenu();

    QAction* m_dictateAction = nullptr;
    QAction* m_cancelAction = nullptr;
    TextInjector* m_injector = nullptr;
#ifdef DICTAPULSE_HAVE_KF6
    KStatusNotifierItem* m_tray = nullptr;
#endif
};

} // namespace dictapulse
