#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    // Each manager is parented to SettingsManager so Qt's object tree
    // cleans them up automatically when SettingsManager is destroyed.
    , m_wifi(new WifiManager(this))
// , m_bluetooth(new BluetoothManager(this))   // ← add when ready
// , m_volume(new VolumeManager(this))
// , m_theme(new ThemeManager(this))
{}
