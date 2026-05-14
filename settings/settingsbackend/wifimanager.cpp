#include "wifimanager.h"

#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>

// -------------------------------------------------------------------------
// WifiNetworkModel
// -------------------------------------------------------------------------

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WifiNetworkModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_networks.count();
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_networks.count())
        return QVariant();

    const WifiNetwork &net = m_networks.at(index.row());
    switch (role) {
    case SsidRole:       return net.ssid;
    case StrengthRole:   return net.strength;
    case SecuredRole:    return net.secured;
    case ConnectedRole:  return net.connected;
    case ApPathRole:     return net.apPath;
    }
    return QVariant();
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SsidRole]      = "ssid";
    roles[StrengthRole]  = "strength";
    roles[SecuredRole]   = "secured";
    roles[ConnectedRole] = "connected";
    roles[ApPathRole]    = "apPath";
    return roles;
}

void WifiNetworkModel::setNetworks(const QList<WifiNetwork> &networks)
{
    beginResetModel();
    m_networks = networks;
    endResetModel();
}

void WifiNetworkModel::updateConnectionState(const QString &connectedSsid)
{
    for (int i = 0; i < m_networks.size(); ++i) {
        bool wasConnected = m_networks[i].connected;
        m_networks[i].connected = (m_networks[i].ssid == connectedSsid);
        if (wasConnected != m_networks[i].connected) {
            emit dataChanged(index(i), index(i), {ConnectedRole});
        }
    }
}

QList<WifiNetwork> WifiNetworkModel::networks() const
{
    return m_networks;
}

// -------------------------------------------------------------------------
// WifiManager
// -------------------------------------------------------------------------

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
{
    initDBusMonitoring();

    // Poll every 2 s so we always reflect external changes (nmcli, system settings, etc.)
    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(2000);
    connect(m_syncTimer, &QTimer::timeout, this, &WifiManager::syncState);
    m_syncTimer->start();
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

QString WifiManager::connectedSsid() const
{
    return m_connectedSsid;
}

WifiNetworkModel *WifiManager::networks()
{
    return &m_networks;
}

void WifiManager::initDBusMonitoring()
{
    bool ok = QDBusConnection::systemBus().connect(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"),
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
        );

    if (!ok)
        qWarning() << "[WifiManager] Failed to connect to NM PropertiesChanged signal";

    // Initial read
    syncState();
}

void WifiManager::syncState()
{
    // 1. Sync on/off state
    QDBusInterface nm(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QDBusConnection::systemBus()
        );

    bool actualEnabled = nm.property("WirelessEnabled").toBool();
    if (m_enabled != actualEnabled) {
        m_enabled = actualEnabled;
        emit enabledChanged();
        m_statusText = m_enabled ? QStringLiteral("WiFi Enabled") : QStringLiteral("WiFi Disabled");
        emit statusTextChanged();
    }

    // 2. Sync active connection (connected SSID, button state)
    updateActiveConnection();
}

void WifiManager::onPropertiesChanged(const QString &interfaceName,
                                      const QVariantMap &changedProperties,
                                      const QStringList &)
{
    if (interfaceName != QLatin1String("org.freedesktop.NetworkManager"))
        return;

    if (changedProperties.contains(QStringLiteral("WirelessEnabled")) ||
        changedProperties.contains(QStringLiteral("ActiveConnections")) ||
        changedProperties.contains(QStringLiteral("State"))) {
        syncState();
    }
}

void WifiManager::updateActiveConnection()
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        if (!m_connectedSsid.isEmpty()) {
            m_connectedSsid.clear();
            emit connectedSsidChanged();
            m_networks.updateConnectionState(QString());
        }
        return;
    }

    QDBusInterface device(
        QStringLiteral("org.freedesktop.NetworkManager"),
        wifiPath,
        QStringLiteral("org.freedesktop.NetworkManager.Device"),
        QDBusConnection::systemBus()
        );

    // Device state: 100 = ACTIVATED
    uint devState = device.property("State").toUInt();

    QDBusObjectPath activeConnPath = device.property("ActiveConnection").value<QDBusObjectPath>();
    if (devState != 100 || activeConnPath.path().isEmpty()) {
        if (!m_connectedSsid.isEmpty()) {
            m_connectedSsid.clear();
            emit connectedSsidChanged();
            m_networks.updateConnectionState(QString());
        }
        return;
    }

    QDBusInterface activeConn(
        QStringLiteral("org.freedesktop.NetworkManager"),
        activeConnPath.path(),
        QStringLiteral("org.freedesktop.NetworkManager.Connection.Active"),
        QDBusConnection::systemBus()
        );

    QDBusObjectPath connPath = activeConn.property("Connection").value<QDBusObjectPath>();
    if (connPath.path().isEmpty()) {
        if (!m_connectedSsid.isEmpty()) {
            m_connectedSsid.clear();
            emit connectedSsidChanged();
            m_networks.updateConnectionState(QString());
        }
        return;
    }

    QDBusInterface conn(
        QStringLiteral("org.freedesktop.NetworkManager"),
        connPath.path(),
        QStringLiteral("org.freedesktop.NetworkManager.Settings.Connection"),
        QDBusConnection::systemBus()
        );

    // GetSettings returns a{sa{sv}}.  Use QDBusMessage to avoid template guesswork.
    QDBusMessage msg = conn.call(QStringLiteral("GetSettings"));
    if (msg.type() == QDBusMessage::ReplyMessage && !msg.arguments().isEmpty()) {
        QVariantMap settings = msg.arguments().first().toMap();
        QVariantMap wireless = settings.value(QStringLiteral("802-11-wireless")).toMap();
        QByteArray ssidBytes = wireless.value(QStringLiteral("ssid")).toByteArray();
        QString ssid = QString::fromUtf8(ssidBytes);

        if (m_connectedSsid != ssid) {
            m_connectedSsid = ssid;
            emit connectedSsidChanged();
            m_networks.updateConnectionState(ssid);
        }
    } else {
        qWarning() << "[WifiManager] GetSettings failed:" << msg.errorMessage();
    }
}

void WifiManager::setEnabled(bool enabled)
{
    QDBusInterface properties(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QDBusConnection::systemBus()
        );

    properties.call(
        QStringLiteral("Set"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("WirelessEnabled"),
        QVariant::fromValue(QDBusVariant(enabled))
        );

    // Optimistic update; syncState() will confirm via timer shortly
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged();
        m_statusText = enabled ? QStringLiteral("WiFi Enabled") : QStringLiteral("WiFi Disabled");
        emit statusTextChanged();
    }
}

QString WifiManager::findWifiDevicePath()
{
    if (!m_wifiDevicePath.isEmpty())
        return m_wifiDevicePath;

    QDBusInterface nm(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QDBusConnection::systemBus()
        );

    QDBusReply<QList<QDBusObjectPath>> reply = nm.call(QStringLiteral("GetDevices"));
    if (!reply.isValid())
        return QString();

    for (const auto &devicePath : reply.value()) {
        QDBusInterface device(
            QStringLiteral("org.freedesktop.NetworkManager"),
            devicePath.path(),
            QStringLiteral("org.freedesktop.NetworkManager.Device"),
            QDBusConnection::systemBus()
            );

        if (device.property("DeviceType").toUInt() == 2) { // 2 = Wi-Fi
            m_wifiDevicePath = devicePath.path();
            return m_wifiDevicePath;
        }
    }

    return QString();
}

void WifiManager::scan()
{
    if (!m_enabled) {
        m_statusText = QStringLiteral("WiFi is disabled");
        emit statusTextChanged();
        return;
    }

    m_scanning = true;
    emit scanningChanged();

    m_statusText = QStringLiteral("Scanning...");
    emit statusTextChanged();

    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        m_statusText = QStringLiteral("No WiFi device found");
        emit statusTextChanged();
        m_scanning = false;
        emit scanningChanged();
        return;
    }

    QDBusInterface wifi(
        QStringLiteral("org.freedesktop.NetworkManager"),
        wifiPath,
        QStringLiteral("org.freedesktop.NetworkManager.Device.Wireless"),
        QDBusConnection::systemBus()
        );

    wifi.call(QStringLiteral("RequestScan"), QVariantMap());

    QDBusReply<QList<QDBusObjectPath>> apReply = wifi.call(QStringLiteral("GetAccessPoints"));
    QList<WifiNetwork> networks;

    for (const auto &apPath : apReply.value()) {
        QDBusInterface ap(
            QStringLiteral("org.freedesktop.NetworkManager"),
            apPath.path(),
            QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
            QDBusConnection::systemBus()
            );

        WifiNetwork network;
        network.ssid     = QString::fromUtf8(ap.property("Ssid").toByteArray());
        network.strength = ap.property("Strength").toInt();
        network.apPath   = apPath.path();

        uint flags    = ap.property("Flags").toUInt();
        uint wpaFlags = ap.property("WpaFlags").toUInt();
        uint rsnFlags = ap.property("RsnFlags").toUInt();
        network.secured = (wpaFlags != 0 || rsnFlags != 0 || (flags & 0x1));

        if (!network.ssid.isEmpty())
            networks.append(network);
    }

    m_networks.setNetworks(networks);
    m_networks.updateConnectionState(m_connectedSsid);

    m_statusText = QStringLiteral("Found %1 networks").arg(networks.count());
    emit statusTextChanged();

    m_scanning = false;
    emit scanningChanged();
}

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        emit connectionResult(false, QStringLiteral("No WiFi device found"));
        return;
    }

    QString apPath;
    bool isSecured = false;
    for (const auto &net : m_networks.networks()) {
        if (net.ssid == ssid) {
            apPath = net.apPath;
            isSecured = net.secured;
            break;
        }
    }

    QVariantMap connection;
    connection[QStringLiteral("id")]   = ssid;
    connection[QStringLiteral("type")] = QStringLiteral("802-11-wireless");

    QVariantMap wireless;
    wireless[QStringLiteral("ssid")]  = QVariant::fromValue(QByteArray(ssid.toUtf8()));
    wireless[QStringLiteral("mode")] = QStringLiteral("infrastructure");

    QVariantMap wirelessSecurity;
    if (isSecured && !password.isEmpty()) {
        wirelessSecurity[QStringLiteral("key-mgmt")] = QStringLiteral("wpa-psk");
        wirelessSecurity[QStringLiteral("psk")]      = password;
    }

    QVariantMap ipv4;
    ipv4[QStringLiteral("method")] = QStringLiteral("auto");

    QVariantMap ipv6;
    ipv6[QStringLiteral("method")] = QStringLiteral("auto");

    QMap<QString, QVariantMap> settings;
    settings[QStringLiteral("connection")] = connection;
    settings[QStringLiteral("802-11-wireless")] = wireless;
    if (!wirelessSecurity.isEmpty())
        settings[QStringLiteral("802-11-wireless-security")] = wirelessSecurity;
    settings[QStringLiteral("ipv4")] = ipv4;
    settings[QStringLiteral("ipv6")] = ipv6;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("AddAndActivateConnection")
        );

    msg << QVariant::fromValue(settings)
        << QVariant::fromValue(QDBusObjectPath(wifiPath))
        << QVariant::fromValue(QDBusObjectPath(apPath));

    QDBusReply<QDBusObjectPath> reply = QDBusConnection::systemBus().call(msg);
    if (reply.isValid()) {
        m_statusText = QStringLiteral("Connecting to %1...").arg(ssid);
        emit statusTextChanged();
        emit connectionResult(true, QStringLiteral("Connecting to %1").arg(ssid));
    } else {
        emit connectionResult(false, QStringLiteral("Failed to connect: %1").arg(reply.error().message()));
    }
}

void WifiManager::disconnect()
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty())
        return;

    QDBusInterface device(
        QStringLiteral("org.freedesktop.NetworkManager"),
        wifiPath,
        QStringLiteral("org.freedesktop.NetworkManager.Device"),
        QDBusConnection::systemBus()
        );

    QDBusObjectPath activeConn = device.property("ActiveConnection").value<QDBusObjectPath>();
    if (activeConn.path().isEmpty()) {
        emit connectionResult(false, QStringLiteral("Not connected"));
        return;
    }

    QDBusInterface nm(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager"),
        QStringLiteral("org.freedesktop.NetworkManager"),
        QDBusConnection::systemBus()
        );

    QDBusReply<void> reply = nm.call(QStringLiteral("DeactivateConnection"),
                                     QVariant::fromValue(activeConn));
    if (reply.isValid())
        emit connectionResult(true, QStringLiteral("Disconnected"));
    else
        emit connectionResult(false, QStringLiteral("Failed to disconnect: %1").arg(reply.error().message()));
}

void WifiManager::forgetNetwork(const QString &ssid)
{
    QDBusInterface settings(
        QStringLiteral("org.freedesktop.NetworkManager"),
        QStringLiteral("/org/freedesktop/NetworkManager/Settings"),
        QStringLiteral("org.freedesktop.NetworkManager.Settings"),
        QDBusConnection::systemBus()
        );

    QDBusReply<QList<QDBusObjectPath>> reply = settings.call(QStringLiteral("ListConnections"));
    if (!reply.isValid()) {
        emit connectionResult(false, QStringLiteral("Failed to list connections"));
        return;
    }

    for (const auto &path : reply.value()) {
        QDBusInterface conn(
            QStringLiteral("org.freedesktop.NetworkManager"),
            path.path(),
            QStringLiteral("org.freedesktop.NetworkManager.Settings.Connection"),
            QDBusConnection::systemBus()
            );

        QDBusMessage msg = conn.call(QStringLiteral("GetSettings"));
        if (msg.type() != QDBusMessage::ReplyMessage || msg.arguments().isEmpty())
            continue;

        QVariantMap allSettings = msg.arguments().first().toMap();
        QVariantMap wireless = allSettings.value(QStringLiteral("802-11-wireless")).toMap();
        QByteArray connSsid = wireless.value(QStringLiteral("ssid")).toByteArray();

        if (QString::fromUtf8(connSsid) == ssid) {
            QDBusReply<void> deleteReply = conn.call(QStringLiteral("Delete"));
            if (deleteReply.isValid()) {
                emit connectionResult(true, QStringLiteral("Forgot %1").arg(ssid));
                if (m_connectedSsid == ssid) {
                    m_connectedSsid.clear();
                    emit connectedSsidChanged();
                    m_networks.updateConnectionState(QString());
                }
                scan();
            } else {
                emit connectionResult(false, QStringLiteral("Failed to forget: %1").arg(deleteReply.error().message()));
            }
            return;
        }
    }

    emit connectionResult(false, QStringLiteral("Network %1 not found in saved connections").arg(ssid));
}
