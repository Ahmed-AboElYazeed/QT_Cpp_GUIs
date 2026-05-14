#include "bluetoothmanager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDebug>

static const QString BZ_SERVICE  = QStringLiteral("org.bluez");
static const QString BZ_ADAPTER  = QStringLiteral("org.bluez.Adapter1");
static const QString BZ_DEVICE   = QStringLiteral("org.bluez.Device1");
static const QString DBUS_OBJMGR = QStringLiteral("org.freedesktop.DBus.ObjectManager");
static const QString DBUS_PROP   = QStringLiteral("org.freedesktop.DBus.Properties");

// ── Unwrap QDBusVariant if needed ─────────────────────────────────────────────
static QVariant unwrap(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<QDBusVariant>())
        return v.value<QDBusVariant>().variant();
    return v;
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

    QDBusInterface adapterIface(BZ_SERVICE, m_adapterPath,
                                BZ_ADAPTER, bus);
    m_enabled     = adapterIface.property("Powered").toBool();
    m_discovering = adapterIface.property("Discovering").toBool();

    emit enabledChanged();
    emit discoveringChanged();
    updateStatus();

    bus.connect(BZ_SERVICE, m_adapterPath, DBUS_PROP,
                "PropertiesChanged", this,
                SLOT(onAdapterPropertiesChanged(QString, QVariantMap, QStringList)));

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
    const ManagedObjects objects = qdbus_cast<ManagedObjects>(reply.arguments().first());

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
    const ManagedObjects objects = qdbus_cast<ManagedObjects>(reply.arguments().first());

    int count = 0;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        const QString path = it.key().path();
        const QMap<QString, QVariantMap> &ifaces = it.value();

        if (ifaces.contains(BZ_DEVICE)) {
            const QVariantMap props = ifaces.value(BZ_DEVICE);
            BluetoothDevice dev = deviceFromDBusProps(path, props);

            qDebug() << "[BT] Found device:" << dev.name
                     << dev.address << "paired:" << dev.paired;

            m_devices.addOrUpdate(dev);
            subscribeToDevice(path);
            ++count;
        }
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

void BluetoothManager::subscribeToDevice(const QString &devicePath)
{
    bool ok = QDBusConnection::systemBus().connect(
        BZ_SERVICE, devicePath, DBUS_PROP,
        "PropertiesChanged", this,
        SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));

    if (!ok)
        qWarning() << "[BT] Failed to subscribe to device:" << devicePath;
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

    QString changedAddress = unwrap(changed.value("Address")).toString();

    QList<BluetoothDevice> list = m_devices.devices();
    for (BluetoothDevice dev : list) {
        bool isMatch = (!changedAddress.isEmpty())
        ? (dev.address == changedAddress)
        : true;

        if (!isMatch) continue;

        if (changed.contains("Connected"))
            dev.connected = unwrap(changed.value("Connected")).toBool();
        if (changed.contains("Paired"))
            dev.paired    = unwrap(changed.value("Paired")).toBool();
        if (changed.contains("Trusted"))
            dev.trusted   = unwrap(changed.value("Trusted")).toBool();
        if (changed.contains("Name"))
            dev.name      = unwrap(changed.value("Name")).toString();

        qDebug() << "[BT] Device updated:" << dev.name
                 << "connected:" << dev.connected
                 << "paired:" << dev.paired;

        m_devices.addOrUpdate(dev);

        if (!changedAddress.isEmpty()) break;
    }

    updateStatus();
}

// ── onInterfacesAdded ─────────────────────────────────────────────────────────

void BluetoothManager::onInterfacesAdded(const QDBusObjectPath &path,
                                         const QVariantMap &interfaces)
{
    if (!interfaces.contains(BZ_DEVICE)) return;

    // Qt6 automatically decodes nested a{sv} into QVariantMap
    QVariantMap props = interfaces.value(BZ_DEVICE).toMap();
    for (auto it = props.begin(); it != props.end(); ++it)
        it.value() = unwrap(it.value());

    BluetoothDevice dev = deviceFromDBusProps(path.path(), props);
    m_devices.addOrUpdate(dev);
    subscribeToDevice(path.path());
    updateStatus();
}

// ── onInterfacesRemoved ───────────────────────────────────────────────────────

void BluetoothManager::onInterfacesRemoved(const QDBusObjectPath &path,
                                           const QStringList &interfaces)
{
    if (!interfaces.contains(BZ_DEVICE)) return;

    for (const BluetoothDevice &dev : m_devices.devices()) {
        if (dev.objectPath == path.path()) {
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
        for (const auto &d : m_devices.devices())
            if (d.connected) ++connected;
        m_statusText = connected > 0
                           ? QString("%1 device(s) connected").arg(connected)
                           : QString("%1 device(s) known").arg(m_devices.devices().size());
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

    if (m_enabled != on) {
        m_enabled = on;
        emit enabledChanged();
        if (!on) {
            m_discovering = false;
            emit discoveringChanged();
            m_devices.clear();
        }
    }
    updateStatus();
}

// ── startDiscovery ────────────────────────────────────────────────────────────

void BluetoothManager::startDiscovery()
{
    if (!m_enabled || m_adapterPath.isEmpty()) return;

    QDBusInterface adapter(BZ_SERVICE, m_adapterPath, BZ_ADAPTER,
                           QDBusConnection::systemBus());
    QDBusReply<void> reply = adapter.call("StartDiscovery");

    if (!reply.isValid())
        emit operationResult(false, "Scan failed: " + reply.error().message());
}

// ── stopDiscovery ─────────────────────────────────────────────────────────────

void BluetoothManager::stopDiscovery()
{
    if (m_adapterPath.isEmpty()) return;

    QDBusInterface adapter(BZ_SERVICE, m_adapterPath, BZ_ADAPTER,
                           QDBusConnection::systemBus());
    adapter.call("StopDiscovery");
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
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, address, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();

                if (reply.isError()) {
                    QString err = reply.error().name();
                    if (err == "org.bluez.Error.AlreadyExists")
                        emit operationResult(true, "Already paired");
                    else
                        emit operationResult(false, "Pairing failed: " + reply.error().message());
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
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, address, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();

                if (reply.isError())
                    emit operationResult(false, "Connect failed: " + reply.error().message());
                else
                    emit operationResult(true, "Connected");
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
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(pending, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, watcher]() {
                QDBusPendingReply<void> reply = *watcher;
                watcher->deleteLater();

                if (reply.isError())
                    emit operationResult(false, "Disconnect failed: " + reply.error().message());
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
        emit operationResult(true, "Device removed");
    } else {
        emit operationResult(false, "Remove failed: " + reply.error().message());
    }
}
