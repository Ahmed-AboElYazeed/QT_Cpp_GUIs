#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "wifi/WifiManager.h"
#include "bluetooth/BluetoothManager.h"
#include "settings/SettingsManager.h"

class AppController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(WifiManager*      wifi      READ wifi      CONSTANT)
    Q_PROPERTY(BluetoothManager* bluetooth READ bluetooth CONSTANT)
    Q_PROPERTY(SettingsManager*  settings  READ settings  CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    WifiManager*      wifi()      const { return m_wifi; }
    BluetoothManager* bluetooth() const { return m_bluetooth; }
    SettingsManager*  settings()  const { return m_settings; }

private:
    WifiManager*      m_wifi;
    BluetoothManager* m_bluetooth;
    SettingsManager*  m_settings;
};

#endif // APPCONTROLLER_H
