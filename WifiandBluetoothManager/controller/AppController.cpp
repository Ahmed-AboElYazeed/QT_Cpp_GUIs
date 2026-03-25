#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_wifi(new WifiManager(this))
    , m_bluetooth(new BluetoothManager(this))
    , m_settings(new SettingsManager(this))
{
    // Forward errors from managers up to QML via AppController if needed
    // e.g.: connect(m_wifi, &WifiManager::connectionError, this, &AppController::onError);
}
