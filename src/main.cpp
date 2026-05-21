#include "app/Controller.h"
#include "app/Settings.h"
#include "core/hardware/HardwareInfo.h"
#include "core/models/ModelManager.h"
#include "platform/linux/LinuxAdapter.h"

#include <QApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char* argv[])
{
    // qInfo() is suppressed by default in release; we use it for the
    // [DictaPulse] diagnostic lines that drive bug reports. Force-on
    // unless the user has explicitly set QT_LOGGING_RULES themselves.
    if (qEnvironmentVariableIsEmpty("QT_LOGGING_RULES")) {
        QLoggingCategory::setFilterRules(QStringLiteral("*.info=true"));
    }

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

    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    auto* settings = new dictapulse::Settings(&app);
    auto* hardware = new dictapulse::HardwareInfo(&app);
    auto* models = new dictapulse::ModelManager(&app);
    auto* platform = new dictapulse::LinuxAdapter(&app);
    platform->setAnimationEnabled(settings->trayIconAnimation());
    QObject::connect(settings, &dictapulse::Settings::trayIconAnimationChanged,
                     platform, [platform, settings]() {
                         platform->setAnimationEnabled(settings->trayIconAnimation());
                     });
    auto* controller = new dictapulse::Controller(settings, models, hardware, platform, &app);

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

    return app.exec();
}
