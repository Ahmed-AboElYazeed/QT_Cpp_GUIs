#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>
#include "controller/AppController.h"

int main(int argc, char *argv[])
{
    qputenv("QT_BLUETOOTH_BACKEND", "bluez");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    AppController controller;

    // Single injection point — QML only ever sees "app"
    engine.rootContext()->setContextProperty("app", &controller);

    engine.loadFromModule("WifiandBluetoothManager", "Main");

    return app.exec();
}
