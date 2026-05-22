#include "bluetoothmanager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDebug>
#include <QTimer>

static const QString BZ_SERVICE  = QStringLiteral("org.bluez");
static const QString BZ_ADAPTER  = QStringLiteral("org.bluez.Adapter1");
static const QString BZ_DEVICE   = QStringLiteral("org.bluez.Device1");
static const QString DBUS_OBJMGR = QStringLiteral("org.freedesktop.DBus.ObjectManager");
static const QString DBUS_PROP   = QStringLiteral("org.freedesktop.DBus.Properties");

static QVariant unwrap(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<QDBusVariant>())
        return v.value<QDBusVariant>().variant();
    return v;
}

// Read ALL properties of a D-Bus interface fresh from the bus.
// This is the single reliable source — never trust signal payloads for
// initial device data because BlueZ populates them after the signal fires.
static QVariantMap readPropsFromDBus(const QString &path,
                                     const QString &interface)
{
    QDBusInterface propsIface(BZ_SERVICE, path, DBUS_PROP,
                              QDBusConnection::systemBus());
    if (!propsIface.isValid()) return {};

    QDBusMessage reply = propsIface.call("GetAll", interface);
    if (reply.type() != QDBusMessage::ReplyMessage
        || reply.arguments().isEmpty())
        return {};

    // GetAll returns a{sv} — qdbus_cast handles this correctly
    return qdbus_cast<QVariantMap>(reply.arguments().first());
}

// ─────────────────────────────────────────────────────────────────────────────
// BluetoothDeviceModel
// ─────────────────────────────────────────────────────────────────────────────

BluetoothDeviceModel::BluetoothDeviceModel(QObject *parent)
    : QAbstractListModel(parent) {}

int BluetoothDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_devices.count();
}

QVariant BluetoothDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_devices.count())
        return {};
    const BluetoothDevice &d = m_devices.at(index.row());
    switch (role) {
    case NameRole:       return d.name.isEmpty() ? d.address : d.name;
    case AddressRole:    return d.address;
    case ObjectPathRole: return d.objectPath;
    case PairedRole:     return d.paired;
    case ConnectedRole:  return d.connected;
    case TrustedRole:    return d.trusted;
    case TypeHintRole:   return d.typeHint();
    }
    return {};
}

QHash<int, QByteArray> BluetoothDeviceModel::roleNames() const
{
    return {
            { NameRole,       "name"       },
            { AddressRole,    "address"    },
            { ObjectPathRole, "objectPath" },
            { PairedRole,     "paired"     },
            { ConnectedRole,  "connected"  },
            { TrustedRole,    "trusted"    },
            { TypeHintRole,   "typeHint"   },
            };
}

void BluetoothDeviceModel::addOrUpdate(const BluetoothDevice &device)
{
    if (device.address.isEmpty()) return;

    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].address == device.address) {
            // FIX: only emit dataChanged if something actually changed
            // This stops the model from signaling QML on every heartbeat
            if (m_devices[i].name      == device.name      &&
                m_devices[i].paired    == device.paired    &&
                m_devices[i].connected == device.connected &&
                m_devices[i].trusted   == device.trusted)
                return;                 // nothing changed — skip signal
            m_devices[i] = device;
            emit dataChanged(index(i), index(i));
            return;
        }
    }
    beginInsertRows({}, m_devices.size(), m_devices.size());
    m_devices.append(device);
    endInsertRows();
}

void BluetoothDeviceModel::remove(const QString &address)
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].address == address) {
            beginRemoveRows({}, i, i);
            m_devices.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void BluetoothDeviceModel::clear()
{
    beginResetModel();
    m_devices.clear();
    endResetModel();
}

QList<BluetoothDevice> BluetoothDeviceModel::devices() const { return m_devices; }

// ─────────────────────────────────────────────────────────────────────────────
// BluetoothManager
// ─────────────────────────────────────────────────────────────────────────────

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    initDBus();
}

bool                  BluetoothManager::enabled()     const { return m_enabled;     }
bool                  BluetoothManager::discovering() const { return m_discovering; }
QString               BluetoothManager::statusText()  const { return m_statusText;  }
BluetoothDeviceModel* BluetoothManager::devices()           { return &m_devices;    }

// ── initDBus ─────────────────────────────────────────────────────────────────

void BluetoothManager::initDBus()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "[BT] Cannot connect to D-Bus system bus";
        m_statusText = "D-Bus unavailable";
        emit statusTextChanged();
        return;
    }

    m_adapterPath = findAdapterPath();
    if (m_adapterPath.isEmpty()) {
        qWarning() << "[BT] No Bluetooth adapter found";
        m_statusText = "No Bluetooth adapter";
        emit statusTextChanged();
        return;
    }

    qDebug() << "[BT] Adapter found at:" << m_adapterPath;

    // Read initial adapter state directly — always use fresh D-Bus read
    QVariantMap adapterProps = readPropsFromDBus(m_adapterPath, BZ_ADAPTER);
    m_enabled     = unwrap(adapterProps.value("Powered")).toBool();
    m_discovering = unwrap(adapterProps.value("Discovering")).toBool();

    emit enabledChanged();
    emit discoveringChanged();
    updateStatus();

    // Adapter property changes (Powered, Discovering)
    bus.connect(BZ_SERVICE, m_adapterPath, DBUS_PROP,
                "PropertiesChanged", this,
                SLOT(onAdapterPropertiesChanged(QString, QVariantMap, QStringList)));

    // Device appear / disappear
    bus.connect(BZ_SERVICE, "/", DBUS_OBJMGR,
                "InterfacesAdded", this,
                SLOT(onInterfacesAdded(QDBusObjectPath, QVariantMap)));
    bus.connect(BZ_SERVICE, "/", DBUS_OBJMGR,
                "InterfacesRemoved", this,
                SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));

    loadExistingDevices();
}

// ── findAdapterPath ───────────────────────────────────────────────────────────

QString BluetoothManager::findAdapterPath()
{
    QDBusInterface objMgr(BZ_SERVICE, "/", DBUS_OBJMGR,
                          QDBusConnection::systemBus());
    QDBusMessage reply = objMgr.call("GetManagedObjects");

    if (reply.type() != QDBusMessage::ReplyMessage
        || reply.arguments().isEmpty()) {
        qWarning() << "[BT] GetManagedObjects failed:" << reply.errorMessage();
        return {};
    }

    using ManagedObjects = QMap<QDBusObjectPath, QMap<QString, QVariantMap>>;
    const ManagedObjects objects =
        qdbus_cast<ManagedObjects>(reply.arguments().first());

    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (it.value().contains(BZ_ADAPTER))
            return it.key().path();
    }
    return {};
}

// ── loadExistingDevices ───────────────────────────────────────────────────────

void BluetoothManager::loadExistingDevices()
{
    QDBusInterface objMgr(BZ_SERVICE, "/", DBUS_OBJMGR,
                          QDBusConnection::systemBus());
    QDBusMessage reply = objMgr.call("GetManagedObjects");

    if (reply.type() != QDBusMessage::ReplyMessage
        || reply.arguments().isEmpty()) {
        qWarning() << "[BT] GetManagedObjects failed in loadExistingDevices";
        return;
    }

    using ManagedObjects = QMap<QDBusObjectPath, QMap<QString, QVariantMap>>;
    const ManagedObjects objects =
        qdbus_cast<ManagedObjects>(reply.arguments().first());

    int count = 0;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        const QString path = it.key().path();
        if (!it.value().contains(BZ_DEVICE)) continue;

        // FIX: always re-read fresh from D-Bus instead of trusting
        // GetManagedObjects payload — props can be stale or incompletely typed
        QVariantMap freshProps = readPropsFromDBus(path, BZ_DEVICE);
        if (freshProps.isEmpty()) continue;

        BluetoothDevice dev = deviceFromDBusProps(path, freshProps);
        if (dev.address.isEmpty()) continue;    // skip ghosts

        m_devices.addOrUpdate(dev);
        subscribeToDevice(path);
        ++count;
    }

    qDebug() << "[BT] Loaded" << count << "existing devices";
    updateStatus();
}

// ── deviceFromDBusProps ───────────────────────────────────────────────────────

BluetoothDevice BluetoothManager::deviceFromDBusProps(const QString &path,
                                                      const QVariantMap &props) const
{
    BluetoothDevice dev;
    dev.objectPath  = path;
    dev.name        = unwrap(props.value("Name")).toString();
    dev.address     = unwrap(props.value("Address")).toString();
    dev.paired      = unwrap(props.value("Paired")).toBool();
    dev.connected   = unwrap(props.value("Connected")).toBool();
    dev.trusted     = unwrap(props.value("Trusted")).toBool();
    dev.deviceClass = unwrap(props.value("Class")).toUInt();

    qDebug() << "[BT] deviceFromProps path:" << path
             << "name:" << dev.name
             << "addr:" << dev.address
             << "paired:" << dev.paired
             << "connected:" << dev.connected;
    return dev;
}

// ── subscribeToDevice ─────────────────────────────────────────────────────────
// FIX: track subscribed paths in m_subscribedPaths to prevent duplicate
// D-Bus connections — duplicate connections cause the signal storm

void BluetoothManager::subscribeToDevice(const QString &devicePath)
{
    if (m_subscribedPaths.contains(devicePath)) return;  // already subscribed

    bool ok = QDBusConnection::systemBus().connect(
        BZ_SERVICE, devicePath, DBUS_PROP,
        "PropertiesChanged", this,
        SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));

    if (ok) {
        m_subscribedPaths.insert(devicePath);
        qDebug() << "[BT] Subscribed to device:" << devicePath;
    } else {
        qWarning() << "[BT] Failed to subscribe to device:" << devicePath;
    }
}

// ── onAdapterPropertiesChanged ────────────────────────────────────────────────

void BluetoothManager::onAdapterPropertiesChanged(const QString &interface,
                                                  const QVariantMap &changed,
                                                  const QStringList &)
{
    if (interface != BZ_ADAPTER) return;

    qDebug() << "[BT] Adapter props changed:" << changed.keys();

    if (changed.contains("Powered")) {
        bool newVal = unwrap(changed.value("Powered")).toBool();
        if (m_enabled != newVal) {
            m_enabled = newVal;
            emit enabledChanged();
            if (!m_enabled) {
                m_discovering = false;
                emit discoveringChanged();
                m_devices.clear();
                m_subscribedPaths.clear();  // reset so re-enable re-subscribes
            }
        }
    }

    if (changed.contains("Discovering")) {
        bool newVal = unwrap(changed.value("Discovering")).toBool();
        if (m_discovering != newVal) {
            m_discovering = newVal;
            emit discoveringChanged();
        }
    }

    updateStatus();
}

// ── onDevicePropertiesChanged ─────────────────────────────────────────────────

void BluetoothManager::onDevicePropertiesChanged(const QString &interface,
                                                 const QVariantMap &changed,
                                                 const QStringList &)
{
    if (interface != BZ_DEVICE) return;

    // Only care about these four fields — ignore RSSI, ManufacturerData etc.
    bool connectedChanged = changed.contains("Connected");
    bool pairedChanged    = changed.contains("Paired");
    bool trustedChanged   = changed.contains("Trusted");
    bool nameChanged      = changed.contains("Name");

    if (!connectedChanged && !pairedChanged && !trustedChanged && !nameChanged)
        return;  // FIX: early exit — stops storm from RSSI/signal updates

    // FIX: identify the exact device by re-reading its current state from D-Bus.
    // We find which subscribed device path actually changed by checking which
    // one now has different values than our model has stored.
    // This is O(n * D-Bus calls) but n is small and only fires on real changes.
    for (const QString &path : m_subscribedPaths) {
        QVariantMap freshProps = readPropsFromDBus(path, BZ_DEVICE);
        if (freshProps.isEmpty()) continue;

        BluetoothDevice fresh = deviceFromDBusProps(path, freshProps);
        if (fresh.address.isEmpty()) continue;

        // Find this device in our model
        for (const BluetoothDevice &existing : m_devices.devices()) {
            if (existing.address != fresh.address) continue;

            // addOrUpdate already checks for equality — only emits if changed
            m_devices.addOrUpdate(fresh);
            break;
        }
    }

    updateStatus();
}

// ── onInterfacesAdded ─────────────────────────────────────────────────────────

void BluetoothManager::onInterfacesAdded(const QDBusObjectPath &path,
                                         const QVariantMap &interfaces)
{
    if (!interfaces.contains(BZ_DEVICE)) return;

    // always read fresh from D-Bus with a small delay.
    // BlueZ fires InterfacesAdded before all properties are populated,
    // so the signal's QVariantMap is often empty or has wrong values.
    // The 300ms delay lets BlueZ finish writing the device's properties.
    QString devicePath = path.path();
    QTimer::singleShot(300, this, [this, devicePath]() {
        QVariantMap freshProps = readPropsFromDBus(devicePath, BZ_DEVICE);
        if (freshProps.isEmpty()) {
            qWarning() << "[BT] Still empty after delay:" << devicePath;
            return;
        }

        BluetoothDevice dev = deviceFromDBusProps(devicePath, freshProps);
        if (dev.address.isEmpty()) {
            qWarning() << "[BT] Device has no address:" << devicePath;
            return;
        }

        qDebug() << "[BT] New device appeared:" << dev.name << dev.address;
        m_devices.addOrUpdate(dev);
        subscribeToDevice(devicePath);
        updateStatus();
    });
}

// ── onInterfacesRemoved ───────────────────────────────────────────────────────

void BluetoothManager::onInterfacesRemoved(const QDBusObjectPath &path,
                                           const QStringList &interfaces)
{
    if (!interfaces.contains(BZ_DEVICE)) return;

    m_subscribedPaths.remove(path.path());

    for (const BluetoothDevice &dev : m_devices.devices()) {
        if (dev.objectPath == path.path()) {
            qDebug() << "[BT] Device removed:" << dev.name;
            m_devices.remove(dev.address);
            break;
        }
    }
    updateStatus();
}

// ── updateStatus ──────────────────────────────────────────────────────────────

void BluetoothManager::updateStatus()
{
    if (!m_enabled) {
        m_statusText = "Bluetooth is off";
    } else if (m_discovering) {
        m_statusText = "Scanning for nearby devices...";
    } else {
        int connected = 0;
        int total     = 0;
        for (const auto &d : m_devices.devices()) {
            if (!d.address.isEmpty()) ++total;
            if (d.connected)         ++connected;
        }
        m_statusText = connected > 0
                           ? QString("%1 device(s) connected").arg(connected)
                           : QString("%1 device(s) known").arg(total);
    }
    emit statusTextChanged();
}

// ── setEnabled ────────────────────────────────────────────────────────────────

void BluetoothManager::setEnabled(bool on)
{
    if (m_adapterPath.isEmpty()) return;

    QDBusInterface props(BZ_SERVICE, m_adapterPath, DBUS_PROP,
                         QDBusConnection::systemBus());
    props.call("Set", BZ_ADAPTER, "Powered",
               QVariant::fromValue(QDBusVariant(on)));

    // Update immediately so UI reacts — onAdapterPropertiesChanged confirms later
    if (m_enabled != on) {
        m_enabled = on;
        emit enabledChanged();
        if (!on) {
            m_discovering = false;
            emit discoveringChanged();
            m_devices.clear();
            m_subscribedPaths.clear();
        } else {
            // Re-enabled: reload devices after BlueZ settles (800ms)
            QTimer::singleShot(800, this, &BluetoothManager::loadExistingDevices);
        }
        updateStatus();
    }
}

// ── startDiscovery ────────────────────────────────────────────────────────────

void BluetoothManager::startDiscovery()
{
    if (!m_enabled || m_adapterPath.isEmpty()) return;
    if (m_discovering) return;  // already scanning

    QDBusInterface adapter(BZ_SERVICE, m_adapterPath, BZ_ADAPTER,
                           QDBusConnection::systemBus());
    QDBusReply<void> reply = adapter.call("StartDiscovery");

    if (!reply.isValid())
        emit operationResult(false, "Scan failed: " + reply.error().message());
    // m_discovering updates via onAdapterPropertiesChanged
}

// ── stopDiscovery ─────────────────────────────────────────────────────────────

void BluetoothManager::stopDiscovery()
{
    if (m_adapterPath.isEmpty()) return;
    if (!m_discovering) return;  // FIX: guard — nothing to stop

    QDBusInterface adapter(BZ_SERVICE, m_adapterPath, BZ_ADAPTER,
                           QDBusConnection::systemBus());
    QDBusReply<void> reply = adapter.call("StopDiscovery");

    if (!reply.isValid())
        emit operationResult(false, "Stop failed: " + reply.error().message());
    // m_discovering updates via onAdapterPropertiesChanged
}

// ── devicePathFromAddress ─────────────────────────────────────────────────────

QString BluetoothManager::devicePathFromAddress(const QString &address) const
{
    for (const BluetoothDevice &dev : m_devices.devices()) {
        if (dev.address == address)
            return dev.objectPath;
    }
    qWarning() << "[BT] No path found for address:" << address;
    return {};
}

// ── pairDevice ────────────────────────────────────────────────────────────────

void BluetoothManager::pairDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) {
        emit operationResult(false, "Device not found: " + address);
        return;
    }

    QDBusInterface device(BZ_SERVICE, path, BZ_DEVICE,
                          QDBusConnection::systemBus());
    QDBusPendingCall pending = device.asyncCall("Pair");
    auto *watcher = new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();
                if (reply.isError()) {
                    if (reply.error().name() == "org.bluez.Error.AlreadyExists")
                        emit operationResult(true, "Already paired");
                    else
                        emit operationResult(false, "Pairing failed: " +
                                                        reply.error().message());
                } else {
                    emit operationResult(true, "Paired successfully");
                }
            });
}

// ── connectDevice ─────────────────────────────────────────────────────────────

void BluetoothManager::connectDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) {
        emit operationResult(false, "Device not found: " + address);
        return;
    }

    QDBusInterface device(BZ_SERVICE, path, BZ_DEVICE,
                          QDBusConnection::systemBus());
    QDBusPendingCall pending = device.asyncCall("Connect");
    auto *watcher = new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();
                if (reply.isError())
                    emit operationResult(false, "Connect failed: " +
                                                    reply.error().message());
                else
                    emit operationResult(true, "Connected");
                // UI updates via onDevicePropertiesChanged → Connected: true
            });
}

// ── disconnectDevice ──────────────────────────────────────────────────────────

void BluetoothManager::disconnectDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    QDBusInterface device(BZ_SERVICE, path, BZ_DEVICE,
                          QDBusConnection::systemBus());
    QDBusPendingCall pending = device.asyncCall("Disconnect");
    auto *watcher = new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();
                if (reply.isError())
                    emit operationResult(false, "Disconnect failed: " +
                                                    reply.error().message());
                else
                    emit operationResult(true, "Disconnected");
            });
}

// ── removeDevice ──────────────────────────────────────────────────────────────

void BluetoothManager::removeDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    QDBusInterface adapter(BZ_SERVICE, m_adapterPath, BZ_ADAPTER,
                           QDBusConnection::systemBus());
    QDBusReply<void> reply = adapter.call(
        "RemoveDevice", QVariant::fromValue(QDBusObjectPath(path)));

    if (reply.isValid()) {
        m_devices.remove(address);
        m_subscribedPaths.remove(path);
        emit operationResult(true, "Device removed");
    } else {
        emit operationResult(false, "Remove failed: " +
                                        reply.error().message());
    }
}