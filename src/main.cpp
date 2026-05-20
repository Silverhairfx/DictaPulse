#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("DictaPulse");
    QCoreApplication::setOrganizationDomain("dictapulse.app");
    QCoreApplication::setApplicationName("DictaPulse");
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));

    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("DictaPulse", "Main");

    return app.exec();
}
