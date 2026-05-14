#include "wifimanager.h"

#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
{
}

bool WifiManager::enabled() const
{
    return m_enabled;
}

bool WifiManager::scanning() const
{
    return m_scanning;
}

QString WifiManager::statusText() const
{
    return m_statusText;
}

WifiNetworkModel *WifiManager::networks()
{
    return &m_networks;
}

void WifiManager::setEnabled(bool enabled)
{
    /*
     * NetworkManager uses DBus properties.
     */

    QDBusInterface properties(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus()
        );

    properties.call(
        "Set",
        "org.freedesktop.NetworkManager",
        "WirelessEnabled",
        QVariant::fromValue(QDBusVariant(enabled))
        );

    m_enabled = enabled;

    emit enabledChanged();

    if (enabled)
        m_statusText = "WiFi Enabled";
    else
        m_statusText = "WiFi Disabled";

    emit statusTextChanged();
}

QString WifiManager::findWifiDevicePath()
{
    /*
     * NetworkManager exposes all devices.
     * We search for the first wireless device.
     */

    QDBusInterface nm(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        QDBusConnection::systemBus()
        );

    QDBusReply<QList<QDBusObjectPath>> reply =
        nm.call("GetDevices");

    if (!reply.isValid())
        return {};

    for (const auto &devicePath : reply.value())
    {
        QDBusInterface device(
            "org.freedesktop.NetworkManager",
            devicePath.path(),
            "org.freedesktop.NetworkManager.Device",
            QDBusConnection::systemBus()
            );

        // DeviceType:
        // 2 = WiFi device
        uint deviceType =
            device.property("DeviceType").toUInt();

        if (deviceType == 2)
            return devicePath.path();
    }

    return {};
}

void WifiManager::scan()
{
    m_scanning = true;

    emit scanningChanged();

    m_statusText = "Scanning...";
    emit statusTextChanged();

    QString wifiPath = findWifiDevicePath();

    if (wifiPath.isEmpty())
    {
        m_statusText = "No WiFi device found";

        emit statusTextChanged();

        m_scanning = false;

        emit scanningChanged();

        return;
    }

    /*
     * Wireless device interface
     */
    QDBusInterface wifi(
        "org.freedesktop.NetworkManager",
        wifiPath,
        "org.freedesktop.NetworkManager.Device.Wireless",
        QDBusConnection::systemBus()
        );

    /*
     * Ask NetworkManager to scan
     */
    wifi.call("RequestScan", QVariantMap());

    /*
     * Get access points
     */
    QDBusReply<QList<QDBusObjectPath>> apReply =
        wifi.call("GetAccessPoints");

    QList<WifiNetwork> networks;

    for (const auto &apPath : apReply.value())
    {
        QDBusInterface ap(
            "org.freedesktop.NetworkManager",
            apPath.path(),
            "org.freedesktop.NetworkManager.AccessPoint",
            QDBusConnection::systemBus()
            );

        WifiNetwork network;

        /*
         * SSID is returned as byte array
         */
        QByteArray ssidBytes =
            ap.property("Ssid").toByteArray();

        network.ssid =
            QString::fromUtf8(ssidBytes);

        network.strength =
            ap.property("Strength").toInt();

        if (!network.ssid.isEmpty())
            networks.append(network);
    }

    m_networks.setNetworks(networks);

    m_statusText =
        QString("Found %1 networks")
            .arg(networks.count());

    emit statusTextChanged();

    m_scanning = false;

    emit scanningChanged();
}
