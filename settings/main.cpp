#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "settingsbackend/settingsmanager.h"
// #include "settingsbackend/wifinetwork.h"       // Needed for qRegisterMetaType

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("SettingsApp");
    app.setOrganizationName("YourOrg");

    // ── 1. Register Types for the Meta-Object System ─────────────────────
    // WifiNetwork is passed through signals — Qt's meta-object system needs
    // to know about it before the first signal fires.
    qRegisterMetaType<WifiNetwork>();

    // WifiNetworkModel is used as a Q_PROPERTY type — register it for QML
    qmlRegisterUncreatableType<WifiNetworkModel>(
        "App.Models", 1, 0, "WifiNetworkModel",
        "WifiNetworkModel is created by WifiManager — do not instantiate from QML"
        );

    // ── 2. Instantiate the Backend ────────────────────────────────────────
    SettingsManager settings;

    // ── 3. Initialize Engine and Expose Context Properties ────────────────
    QQmlApplicationEngine engine;

    // Injected globally into the QML engine environment
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("SettingsManager", &settings);

    // ── 4. Connect Error Handling Signals ─────────────────────────────────
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            qCritical() << "Failed to instantiate QML module objects.";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    // ── 5. Load the Project via Modern Qt 6 QML Modules ───────────────────
    // Replaces engine.load(url) to match your working QML module target
    engine.loadFromModule("settings", "Main");

    return app.exec();
}
