#include "wifimanager.h"

#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QTimer>

static const QString NM_SERVICE    = QStringLiteral("org.freedesktop.NetworkManager");
static const QString NM_PATH       = QStringLiteral("/org/freedesktop/NetworkManager");
static const QString NM_IFACE      = QStringLiteral("org.freedesktop.NetworkManager");
static const QString NM_DEV_IFACE  = QStringLiteral("org.freedesktop.NetworkManager.Device");
static const QString NM_DEV_WIFI   = QStringLiteral("org.freedesktop.NetworkManager.Device.Wireless");
static const QString NM_AP_IFACE   = QStringLiteral("org.freedesktop.NetworkManager.AccessPoint");
static const QString NM_SETTINGS   = QStringLiteral("org.freedesktop.NetworkManager.Settings");
static const QString NM_CONN       = QStringLiteral("org.freedesktop.NetworkManager.Settings.Connection");
static const QString DBUS_PROP     = QStringLiteral("org.freedesktop.DBus.Properties");

// ─────────────────────────────────────────────────────────────────────────────
// WifiNetworkModel  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent) {}

int WifiNetworkModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_networks.count();
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_networks.count())
        return QVariant();

    const WifiNetwork &net = m_networks.at(index.row());
    switch (role) {
    case SsidRole:      return net.ssid;
    case StrengthRole:  return net.strength;
    case SecuredRole:   return net.secured;
    case ConnectedRole: return net.connected;
    case ApPathRole:    return net.apPath;
    }
    return QVariant();
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const
{
    return {
            { SsidRole,      "ssid"      },
            { StrengthRole,  "strength"  },
            { SecuredRole,   "secured"   },
            { ConnectedRole, "connected" },
            { ApPathRole,    "apPath"    },
            };
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
        if (wasConnected != m_networks[i].connected)
            emit dataChanged(index(i), index(i), { ConnectedRole });
    }
}

QList<WifiNetwork> WifiNetworkModel::networks() const { return m_networks; }

// ─────────────────────────────────────────────────────────────────────────────
// WifiManager
// ─────────────────────────────────────────────────────────────────────────────

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
{
    initDBusMonitoring();
}

bool             WifiManager::enabled()       const { return m_enabled;       }
bool             WifiManager::scanning()      const { return m_scanning;      }
QString          WifiManager::statusText()    const { return m_statusText;    }
QString          WifiManager::connectedSsid() const { return m_connectedSsid; }
WifiNetworkModel* WifiManager::networks()           { return &m_networks;     }

// ── helpers ──────────────────────────────────────────────────────────────────

// Read WirelessEnabled directly from NM properties — single source of truth
bool WifiManager::readWirelessEnabled() const
{
    QDBusInterface props(NM_SERVICE, NM_PATH, DBUS_PROP,
                         QDBusConnection::systemBus());
    QDBusMessage reply = props.call("Get", NM_IFACE, "WirelessEnabled");
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
        return reply.arguments().first().value<QDBusVariant>().variant().toBool();
    return false;
}

// ── init ─────────────────────────────────────────────────────────────────────

void WifiManager::initDBusMonitoring()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "[WifiManager] Cannot connect to D-Bus system bus";
        return;
    }

    // --- 1. Read initial enabled state ---
    m_enabled = readWirelessEnabled();
    emit enabledChanged();
    m_statusText = m_enabled ? "Wi-Fi Enabled" : "Wi-Fi Disabled";
    emit statusTextChanged();

    // --- 2. Find WiFi device path ---
    m_wifiDevicePath = findWifiDevicePath();

    // --- 3. Watch NM root: PropertiesChanged (WirelessEnabled) ---
    bus.connect(NM_SERVICE, NM_PATH, DBUS_PROP,
                "PropertiesChanged", this,
                SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    // --- 4. Watch NM root: StateChanged ---
    // NM fires this whenever global connectivity changes (wifi on/off,
    // cable plugged, airplane mode). We use it to re-read WirelessEnabled
    // because some NM versions don't fire PropertiesChanged reliably.
    bus.connect(NM_SERVICE, NM_PATH, NM_IFACE,
                "StateChanged", this,
                SLOT(onNMStateChanged(uint)));

    // --- 5. Watch the wireless device: PropertiesChanged ---
    // ActiveAccessPoint lives on NM_DEV_WIFI — we need this path, not NM_DEV_IFACE
    if (!m_wifiDevicePath.isEmpty()) {
        bus.connect(NM_SERVICE, m_wifiDevicePath, DBUS_PROP,
                    "PropertiesChanged", this,
                    SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

        // Also watch the device-level StateChanged (connected/disconnected events)
        bus.connect(NM_SERVICE, m_wifiDevicePath, NM_DEV_IFACE,
                    "StateChanged", this,
                    SLOT(onDeviceStateChanged(uint, uint, uint)));
    }

    // --- 6. Initial network load ---
    if (m_enabled)
        QTimer::singleShot(300, this, &WifiManager::refreshNetworks);
}

// ── D-Bus slots ──────────────────────────────────────────────────────────────

// FIX 1: also handle NM_DEV_WIFI interface for ActiveAccessPoint changes
void WifiManager::onPropertiesChanged(const QString &interfaceName,
                                      const QVariantMap &changedProperties,
                                      const QStringList &)
{
    // A. NM root: WirelessEnabled toggled externally
    if (interfaceName == NM_IFACE
        && changedProperties.contains("WirelessEnabled"))
    {
        QVariant v = changedProperties.value("WirelessEnabled");
        bool newEnabled = (v.userType() == qMetaTypeId<QDBusVariant>())
                              ? v.value<QDBusVariant>().variant().toBool()
                              : v.toBool();

        if (m_enabled != newEnabled) {
            m_enabled = newEnabled;
            emit enabledChanged();
            m_statusText = newEnabled ? "Wi-Fi Enabled" : "Wi-Fi Disabled";
            emit statusTextChanged();
            if (newEnabled)
                QTimer::singleShot(300, this, &WifiManager::refreshNetworks);
            else
                m_networks.setNetworks({});  // clear list when disabled
        }
    }

    // B. Wireless device: ActiveAccessPoint changed — re-check connected state
    // FIX 2: use NM_DEV_WIFI here, not NM_DEV_IFACE
    if (interfaceName == NM_DEV_WIFI
        && changedProperties.contains("ActiveAccessPoint"))
    {
        refreshNetworks();
    }
}

// FIX 1 continued: NM global state changed — re-read WirelessEnabled
// This catches cases where PropertiesChanged is not fired (some NM versions)
void WifiManager::onNMStateChanged(uint /*newState*/)
{
    bool newEnabled = readWirelessEnabled();
    if (m_enabled != newEnabled) {
        m_enabled = newEnabled;
        emit enabledChanged();
        m_statusText = newEnabled ? "Wi-Fi Enabled" : "Wi-Fi Disabled";
        emit statusTextChanged();
        if (newEnabled)
            QTimer::singleShot(300, this, &WifiManager::refreshNetworks);
        else
            m_networks.setNetworks({});
    }
}

// FIX 2 continued: device connection state changed (40=disconnected, 100=connected)
void WifiManager::onDeviceStateChanged(uint newState, uint /*oldState*/, uint /*reason*/)
{
    // 100 = NM_DEVICE_STATE_ACTIVATED (connected)
    // 30  = NM_DEVICE_STATE_DISCONNECTED
    // 40  = NM_DEVICE_STATE_PREPARE (connecting...)
    if (newState == 100 || newState == 30) {
        // Small delay to let NM update ActiveAccessPoint before we read it
        QTimer::singleShot(500, this, &WifiManager::refreshNetworks);
    }
}

// ── setEnabled ───────────────────────────────────────────────────────────────

void WifiManager::setEnabled(bool enabled)
{
    QDBusInterface props(NM_SERVICE, NM_PATH, DBUS_PROP,
                         QDBusConnection::systemBus());
    props.call("Set", NM_IFACE, "WirelessEnabled",
               QVariant::fromValue(QDBusVariant(enabled)));

    // Don't update m_enabled here — wait for the PropertiesChanged signal
    // so that the UI only updates when NM confirms the change.
    // This prevents the toggle from snapping back if NM rejects the call.
}

// ── findWifiDevicePath ───────────────────────────────────────────────────────

QString WifiManager::findWifiDevicePath()
{
    if (!m_wifiDevicePath.isEmpty())
        return m_wifiDevicePath;

    QDBusInterface nm(NM_SERVICE, NM_PATH, NM_IFACE,
                      QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> reply = nm.call("GetDevices");
    if (!reply.isValid()) return {};

    for (const auto &devicePath : reply.value()) {
        QDBusInterface device(NM_SERVICE, devicePath.path(),
                              NM_DEV_IFACE,
                              QDBusConnection::systemBus());
        if (device.property("DeviceType").toUInt() == 2) {  // 2 = wifi
            m_wifiDevicePath = devicePath.path();
            return m_wifiDevicePath;
        }
    }
    return {};
}

// ── refreshNetworks ──────────────────────────────────────────────────────────

void WifiManager::refreshNetworks()
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        m_scanning = false;
        emit scanningChanged();
        return;
    }

    QDBusInterface wifi(NM_SERVICE, wifiPath, NM_DEV_WIFI,
                        QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> apReply = wifi.call("GetAllAccessPoints");

    if (!apReply.isValid()) {
        qWarning() << "[WifiManager] GetAllAccessPoints failed:"
                   << apReply.error().message();
        m_scanning = false;
        emit scanningChanged();
        return;
    }

    // ── FIX 2: read ActiveAccessPoint from NM_DEV_WIFI, not NM_DEV_IFACE ──
    // This is the core fix — property lives on the Wireless sub-interface
    QString activeApPath = wifi.property("ActiveAccessPoint")
                               .value<QDBusObjectPath>().path();

    qDebug() << "[WifiManager] ActiveAccessPoint path:" << activeApPath;

    QList<WifiNetwork> networks;

    for (const auto &apPath : apReply.value()) {
        QDBusInterface ap(NM_SERVICE, apPath.path(), NM_AP_IFACE,
                          QDBusConnection::systemBus());

        WifiNetwork network;
        network.ssid     = QString::fromUtf8(ap.property("Ssid").toByteArray());
        network.strength = ap.property("Strength").toInt();
        network.apPath   = apPath.path();

        uint wpaFlags = ap.property("WpaFlags").toUInt();
        uint rsnFlags = ap.property("RsnFlags").toUInt();
        uint flags    = ap.property("Flags").toUInt();
        network.secured = (wpaFlags != 0 || rsnFlags != 0 || (flags & 0x1));

        // FIX 2: mark the connected network correctly
        network.connected = (!activeApPath.isEmpty()
                             && network.apPath == activeApPath);

        if (!network.ssid.isEmpty())
            networks.append(network);
    }

    // Sort: connected first, then by signal strength descending
    std::sort(networks.begin(), networks.end(),
              [](const WifiNetwork &a, const WifiNetwork &b) {
                  if (a.connected != b.connected)
                      return a.connected > b.connected;
                  return a.strength > b.strength;
              });

    m_networks.setNetworks(networks);

    // Update connectedSsid
    QString connectedSsid;
    for (const auto &net : networks) {
        if (net.connected) { connectedSsid = net.ssid; break; }
    }

    if (m_connectedSsid != connectedSsid) {
        m_connectedSsid = connectedSsid;
        emit connectedSsidChanged();
    }

    m_statusText = connectedSsid.isEmpty()
                       ? QString("Found %1 networks").arg(networks.count())
                       : QString("Connected to %1").arg(connectedSsid);
    emit statusTextChanged();

    m_scanning = false;
    emit scanningChanged();
}

// ── scan ─────────────────────────────────────────────────────────────────────

void WifiManager::scan()
{
    if (!m_enabled) {
        m_statusText = "Wi-Fi is disabled";
        emit statusTextChanged();
        return;
    }

    m_scanning = true;
    emit scanningChanged();
    m_statusText = "Scanning...";
    emit statusTextChanged();

    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        m_statusText = "No Wi-Fi device found";
        emit statusTextChanged();
        m_scanning = false;
        emit scanningChanged();
        return;
    }

    QDBusInterface wifi(NM_SERVICE, wifiPath, NM_DEV_WIFI,
                        QDBusConnection::systemBus());
    QDBusMessage reply = wifi.call("RequestScan", QVariantMap());

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "[WifiManager] RequestScan error:" << reply.errorMessage();
        // Still refresh — existing cached APs are still valid
        QTimer::singleShot(500, this, &WifiManager::refreshNetworks);
    } else {
        QTimer::singleShot(2000, this, &WifiManager::refreshNetworks);
    }
}

// ── connectToNetwork ─────────────────────────────────────────────────────────

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) {
        emit connectionResult(false, "No Wi-Fi device found");
        return;
    }

    QString apPath;
    bool    isSecured = false;
    for (const auto &net : m_networks.networks()) {
        if (net.ssid == ssid) {
            apPath    = net.apPath;
            isSecured = net.secured;
            break;
        }
    }

    QVariantMap connection;
    connection["id"]   = ssid;
    connection["type"] = "802-11-wireless";

    QVariantMap wireless;
    wireless["ssid"] = ssid.toUtf8();
    wireless["mode"] = "infrastructure";

    QVariantMap wirelessSecurity;
    if (isSecured && !password.isEmpty()) {
        wirelessSecurity["key-mgmt"] = "wpa-psk";
        wirelessSecurity["psk"]      = password;
    }

    QVariantMap ipv4; ipv4["method"] = "auto";
    QVariantMap ipv6; ipv6["method"] = "auto";

    QMap<QString, QVariantMap> settings;
    settings["connection"]          = connection;
    settings["802-11-wireless"]     = wireless;
    if (!wirelessSecurity.isEmpty())
        settings["802-11-wireless-security"] = wirelessSecurity;
    settings["ipv4"] = ipv4;
    settings["ipv6"] = ipv6;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        NM_SERVICE, NM_PATH, NM_IFACE, "AddAndActivateConnection");

    msg << QVariant::fromValue(settings)
        << QVariant::fromValue(QDBusObjectPath(wifiPath))
        << QVariant::fromValue(QDBusObjectPath(apPath));

    QDBusReply<QDBusObjectPath> reply =
        QDBusConnection::systemBus().call(msg);

    if (reply.isValid()) {
        m_statusText = QString("Connecting to %1...").arg(ssid);
        emit statusTextChanged();
        emit connectionResult(true, QString("Connecting to %1").arg(ssid));
        // onDeviceStateChanged will fire when connection completes
        // but also set a fallback timer
        QTimer::singleShot(4000, this, &WifiManager::refreshNetworks);
    } else {
        emit connectionResult(false,
                              QString("Failed to connect: %1").arg(reply.error().message()));
    }
}

// ── disconnect ───────────────────────────────────────────────────────────────

void WifiManager::disconnect()
{
    QString wifiPath = findWifiDevicePath();
    if (wifiPath.isEmpty()) return;

    QDBusInterface device(NM_SERVICE, wifiPath, NM_DEV_IFACE,
                          QDBusConnection::systemBus());
    QDBusObjectPath activeConn =
        device.property("ActiveConnection").value<QDBusObjectPath>();

    if (activeConn.path().isEmpty() || activeConn.path() == "/") {
        emit connectionResult(false, "Not connected");
        return;
    }

    QDBusInterface nm(NM_SERVICE, NM_PATH, NM_IFACE,
                      QDBusConnection::systemBus());
    QDBusReply<void> reply = nm.call("DeactivateConnection",
                                     QVariant::fromValue(activeConn));
    if (reply.isValid()) {
        emit connectionResult(true, "Disconnected");
        QTimer::singleShot(1000, this, &WifiManager::refreshNetworks);
    } else {
        emit connectionResult(false,
                              QString("Failed to disconnect: %1").arg(reply.error().message()));
    }
}

// ── forgetNetwork ────────────────────────────────────────────────────────────

void WifiManager::forgetNetwork(const QString &ssid)
{
    QDBusInterface settings(NM_SERVICE,
                            "/org/freedesktop/NetworkManager/Settings",
                            NM_SETTINGS,
                            QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = settings.call("ListConnections");
    if (!reply.isValid()) {
        emit connectionResult(false, "Failed to list connections");
        return;
    }

    for (const auto &path : reply.value()) {
        QDBusInterface conn(NM_SERVICE, path.path(), NM_CONN,
                            QDBusConnection::systemBus());
        QDBusMessage msg = conn.call("GetSettings");
        if (msg.type() != QDBusMessage::ReplyMessage
            || msg.arguments().isEmpty())
            continue;

        QVariantMap allSettings = msg.arguments().first().toMap();
        QByteArray  connSsid    = allSettings.value("802-11-wireless")
                                  .toMap().value("ssid").toByteArray();

        if (QString::fromUtf8(connSsid) == ssid) {
            QDBusReply<void> del = conn.call("Delete");
            if (del.isValid()) {
                emit connectionResult(true, QString("Forgot %1").arg(ssid));
                if (m_connectedSsid == ssid) {
                    m_connectedSsid.clear();
                    emit connectedSsidChanged();
                }
                QTimer::singleShot(500, this, &WifiManager::refreshNetworks);
            } else {
                emit connectionResult(false,
                                      QString("Failed to forget: %1").arg(del.error().message()));
            }
            return;
        }
    }

    emit connectionResult(false,
                          QString("%1 not found in saved connections").arg(ssid));
}
