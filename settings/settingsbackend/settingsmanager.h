#pragma once
#include <QObject>
#include "settingsbackend/wifimanager.h"

// ── Forward-declare future managers so the header compiles now ─────────────
// Uncomment each class as you implement it in the next messages.
// class BluetoothManager;
// class VolumeManager;
// class ThemeManager;

/**
 * SettingsManager
 * ---------------
 * Single top-level QObject that owns every sub-manager.
 * Register it once in main.cpp and access children from QML.
 *
 * QML usage:
 *   SettingsManager.wifi.enabled = true
 *   SettingsManager.wifi.scan()
 *   SettingsManager.bluetooth.enabled  // when added
 *   SettingsManager.volume.level       // when added
 *   SettingsManager.theme.darkMode     // when added
 *
 * Alternatively expose each manager directly as its own context property —
 * both approaches work.  The aggregated approach here makes it easy to
 * serialise / restore the whole app state in one place.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

    // Each sub-manager is a CONSTANT pointer property — the pointer never
    // changes after construction, only the object's own properties do.
    Q_PROPERTY(WifiManager* wifi
                   READ wifi
                       CONSTANT)

    // ── Uncomment as you add each manager ─────────────────────────────────
    // Q_PROPERTY(BluetoothManager* bluetooth READ bluetooth CONSTANT)
    // Q_PROPERTY(VolumeManager*    volume    READ volume    CONSTANT)
    // Q_PROPERTY(ThemeManager*     theme     READ theme     CONSTANT)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    WifiManager* wifi() const { return m_wifi; }

    // ── Add getters here as you implement each manager ─────────────────────
    // BluetoothManager* bluetooth() const { return m_bluetooth; }
    // VolumeManager*    volume()    const { return m_volume; }
    // ThemeManager*     theme()     const { return m_theme; }

private:
    WifiManager *m_wifi = nullptr;

    // BluetoothManager *m_bluetooth = nullptr;
    // VolumeManager    *m_volume    = nullptr;
    // ThemeManager     *m_theme     = nullptr;
};
