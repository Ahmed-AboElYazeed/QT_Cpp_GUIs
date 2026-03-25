#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>
#include "Calculator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Create the backend object
    Calculator calculator;

    // Inject it into QML under the name "calculator"
    engine.rootContext()->setContextProperty("calculator", &calculator);

    engine.loadFromModule("CalculatorApp", "Main");

    return app.exec();
}
