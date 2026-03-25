#include "WifiManager.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>
#include <QDebug>

#include <QTimer>

static const QString NM_SERVICE    = "org.freedesktop.NetworkManager";
static const QString NM_PATH       = "/org/freedesktop/NetworkManager";
static const QString NM_IFACE      = "org.freedesktop.NetworkManager";
static const QString NM_DEV_WIFI   = "org.freedesktop.NetworkManager.Device.Wireless";
static const QString NM_AP_IFACE   = "org.freedesktop.NetworkManager.AccessPoint";
static const QString DBUS_PROP     = "org.freedesktop.DBus.Properties";

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
    , m_networks(new WifiNetworkModel(this))
{
    initDBus();
}

void WifiManager::initDBus()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "Cannot connect to D-Bus system bus";
        return;
    }

    m_nmIface = new QDBusInterface(NM_SERVICE, NM_PATH, NM_IFACE,
                                   bus, this);
    if (!m_nmIface->isValid()) {
        qWarning() << "NetworkManager D-Bus interface not available";
        return;
    }

    // Check current WiFi state
    QDBusReply<uint> state = m_nmIface->call("state");
    m_enabled = (state.isValid() && state.value() >= 70); // 70 = NM_STATE_CONNECTED_LOCAL

    // Find the wireless device path
    m_devicePath = findWirelessDevice();

    if (!m_devicePath.isEmpty()) {
        m_deviceIface = new QDBusInterface(NM_SERVICE, m_devicePath,
                                           NM_DEV_WIFI, bus, this);

        // Listen for scan results
        bus.connect(NM_SERVICE, m_devicePath, NM_DEV_WIFI,
                    "ScanDone", this, SLOT(onScanDone()));

        // Listen for property changes (state, connectivity)
        bus.connect(NM_SERVICE, NM_PATH, DBUS_PROP,
                    "PropertiesChanged", this,
                    SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    }
}

QString WifiManager::findWirelessDevice()
{
    QDBusReply<QList<QDBusObjectPath>> reply =
        m_nmIface->call("GetDevices");

    if (!reply.isValid()) return {};

    for (const QDBusObjectPath &path : reply.value()) {
        QDBusInterface devIface(NM_SERVICE, path.path(),
                                "org.freedesktop.NetworkManager.Device",
                                QDBusConnection::systemBus());

        QVariant devType = devIface.property("DeviceType");
        if (devType.toUInt() == 2) // 2 = NM_DEVICE_TYPE_WIFI
            return path.path();
    }
    return {};
}

void WifiManager::setEnabled(bool on)
{
    if (!m_nmIface || !m_nmIface->isValid()) return;

    // NetworkManager property: WirelessEnabled
    QDBusInterface nmProps(NM_SERVICE, NM_PATH, DBUS_PROP,
                           QDBusConnection::systemBus());
    nmProps.call("Set", NM_IFACE, "WirelessEnabled", QVariant::fromValue(QDBusVariant(on)));

    m_enabled = on;
    emit enabledChanged();
}

// void WifiManager::startScan()
// {
//     if (!m_deviceIface || !m_deviceIface->isValid()) return;

//     QVariantMap options;
//     m_deviceIface->call("RequestScan", options);
//     // onScanDone() will fire when complete
// }
void WifiManager::startScan()
{
    qDebug() << "=== startScan() called ===";

    if (!m_deviceIface || !m_deviceIface->isValid()) {
        qDebug() << "WiFi Interface is INVALID!";
        return;
    }

    QVariantMap options;
    QDBusMessage reply = m_deviceIface->call("RequestScan", options);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "WiFi Scan Error:" << reply.errorMessage();
    } else {
        qDebug() << "WiFi Scan successfully requested, waiting for results...";

        // FIX: Call refreshNetworks() after 2 seconds delay
        // (since ScanDone signal doesn't exist!)
        QTimer::singleShot(2000, this, &WifiManager::refreshNetworks);
    }
}

void WifiManager::onScanDone()
{
    qDebug() << "=== onScanDone() called ===";  // This will NEVER print!
    refreshNetworks();
}

void WifiManager::refreshNetworks()
{
    qDebug() << "=== refreshNetworks() called ===";  // ADD THIS LINE

    if (!m_deviceIface || !m_deviceIface->isValid()) return;

    QDBusReply<QList<QDBusObjectPath>> reply =
        m_deviceIface->call("GetAllAccessPoints");

    if (!reply.isValid()) {
        qDebug() << "GetAllAccessPoints FAILED:" << reply.error().message();  // ADD THIS
        return;
    }

    qDebug() << "Found" << reply.value().size() << "access points";  // ADD THIS

    QList<WifiNetwork> networks;

    for (const QDBusObjectPath &apPath : reply.value()) {
        QDBusInterface apIface(NM_SERVICE, apPath.path(),
                               NM_AP_IFACE,
                               QDBusConnection::systemBus());

        WifiNetwork net;
        net.ssid             = QString::fromUtf8(apIface.property("Ssid").toByteArray());
        net.signalStrength   = apIface.property("Strength").toInt();
        net.isSecured        = (apIface.property("RsnFlags").toUInt() > 0 ||
                         apIface.property("WpaFlags").toUInt() > 0);
        net.isConnected      = false;
        net.accessPointPath  = apPath.path();

        qDebug() << "  Found network:" << net.ssid << "Signal:" << net.signalStrength;  // ADD THIS

        if (!net.ssid.isEmpty())
            networks.append(net);
    }

    // Mark which one is currently active
    QDBusInterface activeIface(NM_SERVICE, m_devicePath,
                               "org.freedesktop.NetworkManager.Device",
                               QDBusConnection::systemBus());
    QString activeApPath = activeIface.property("ActiveAccessPoint")
                               .value<QDBusObjectPath>().path();

    for (auto &net : networks) {
        if (net.accessPointPath == activeApPath)
            net.isConnected = true;
    }

    qDebug() << "Total valid networks:" << networks.size();  // ADD THIS

    m_networks->setNetworks(networks);
    emit networksChanged();
}

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    if (!m_nmIface || !m_nmIface->isValid()) return;

    // Build connection settings dict
    QVariantMap wireless;
    wireless["ssid"] = ssid.toUtf8();
    wireless["mode"] = "infrastructure";

    QVariantMap security;
    QVariantMap ipv4, ipv6;

    if (!password.isEmpty()) {
        security["key-mgmt"] = "wpa-psk";
        security["psk"]      = password;
    }

    ipv4["method"] = "auto";
    ipv6["method"] = "auto";

    QVariantMap connection;
    connection["type"] = "802-11-wireless";
    connection["id"]   = ssid;

    QVariantMap settings;
    settings["connection"]            = connection;
    settings["802-11-wireless"]       = wireless;
    settings["802-11-wireless-security"] = security;
    settings["ipv4"]                  = ipv4;
    settings["ipv6"]                  = ipv6;

    QDBusReply<QDBusObjectPath> reply =
        m_nmIface->call("AddAndActivateConnection",
                        QVariant::fromValue(settings),
                        QVariant::fromValue(QDBusObjectPath(m_devicePath)),
                        QVariant::fromValue(QDBusObjectPath("/")));

    if (!reply.isValid())
        emit connectionError("Failed to connect: " + reply.error().message());
}

void WifiManager::forgetNetwork(const QString &ssid)
{
    // Get all saved connections, find the one matching ssid, remove it
    QDBusInterface settingsIface(NM_SERVICE,
                                 "/org/freedesktop/NetworkManager/Settings",
                                 "org.freedesktop.NetworkManager.Settings",
                                 QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = settingsIface.call("ListConnections");
    if (!reply.isValid()) return;

    for (const QDBusObjectPath &connPath : reply.value()) {
        QDBusInterface connIface(NM_SERVICE, connPath.path(),
                                 "org.freedesktop.NetworkManager.Settings.Connection",
                                 QDBusConnection::systemBus());
        QDBusReply<QVariantMap> settings = connIface.call("GetSettings");
        if (!settings.isValid()) continue;

        QString connSsid = settings.value()
                               .value("802-11-wireless")
                               .toMap()
                               .value("ssid").toString();
        if (connSsid == ssid) {
            connIface.call("Delete");
            refreshNetworks();
            return;
        }
    }
}

void WifiManager::disconnectCurrent()
{
    if (!m_nmIface || !m_nmIface->isValid()) return;
    QDBusInterface devIface(NM_SERVICE, m_devicePath,
                            "org.freedesktop.NetworkManager.Device",
                            QDBusConnection::systemBus());
    devIface.call("Disconnect");
}

void WifiManager::onPropertiesChanged(const QString &interface,
                                      const QVariantMap &changed,
                                      const QStringList &)
{
    if (interface == NM_IFACE && changed.contains("WirelessEnabled")) {
        m_enabled = changed.value("WirelessEnabled").toBool();
        emit enabledChanged();
    }
}
