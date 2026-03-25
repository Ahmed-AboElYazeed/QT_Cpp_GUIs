#include "SettingsManager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_store("WifiBluetoothManager", "settings") {}

bool SettingsManager::darkMode() const
{
    return m_store.value("darkMode", false).toBool();
}

void SettingsManager::setDarkMode(bool on)
{
    m_store.setValue("darkMode", on);
    emit darkModeChanged();
}

void SettingsManager::saveKnownNetwork(const QString &ssid, const QString &password)
{
    m_store.beginGroup("networks");
    m_store.setValue(ssid, password);
    m_store.endGroup();
}

QString SettingsManager::passwordForNetwork(const QString &ssid)
{
    m_store.beginGroup("networks");
    QString pw = m_store.value(ssid).toString();
    m_store.endGroup();
    return pw;
}
