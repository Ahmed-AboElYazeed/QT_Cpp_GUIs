#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "systeminfo.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    SystemInfo sysInfo;
    engine.rootContext()->setContextProperty("systemInfo", &sysInfo);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/HoursesGalaryGUI/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
