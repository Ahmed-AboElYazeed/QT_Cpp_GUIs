#include "BluetoothManager.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QDBusMetaType>
#include <QDebug>

static const QString BLUEZ_SERVICE  = "org.bluez";
static const QString BLUEZ_ADAPTER  = "org.bluez.Adapter1";
static const QString BLUEZ_DEVICE   = "org.bluez.Device1";
static const QString DBUS_OBJMGR    = "org.freedesktop.DBus.ObjectManager";
static const QString DBUS_PROP      = "org.freedesktop.DBus.Properties";

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_devices(new BluetoothDeviceModel(this))
{
    initDBus();
}

void BluetoothManager::initDBus()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "Cannot connect to D-Bus system bus";
        return;
    }

    QString adapterPath = findAdapter();
    if (adapterPath.isEmpty()) {
        qWarning() << "No Bluetooth adapter found via BlueZ";
        return;
    }
    m_adapterPath = QDBusObjectPath(adapterPath);

    m_adapterIface = new QDBusInterface(BLUEZ_SERVICE, adapterPath,
                                        BLUEZ_ADAPTER, bus, this);

    m_enabled = m_adapterIface->property("Powered").toBool();

    // Listen for new/removed devices via ObjectManager
    bus.connect(BLUEZ_SERVICE, "/",
                DBUS_OBJMGR, "InterfacesAdded",
                this,
                SLOT(onInterfacesAdded(QDBusObjectPath, QVariantMap)));

    bus.connect(BLUEZ_SERVICE, "/",
                DBUS_OBJMGR, "InterfacesRemoved",
                this,
                SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));

    // Listen for adapter property changes (Powered, Discovering)
    bus.connect(BLUEZ_SERVICE, adapterPath,
                DBUS_PROP, "PropertiesChanged",
                this,
                SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    loadKnownDevices();
}

QString BluetoothManager::findAdapter()
{
    qDebug() << "=== findAdapter() called ===";

    QDBusConnection bus = QDBusConnection::systemBus();

    // First, check if BlueZ service exists on the bus
    QDBusInterface dbus("org.freedesktop.DBus", "/org/freedesktop/DBus",
                        "org.freedesktop.DBus", bus);
    QDBusReply<bool> nameExists = dbus.call("NameHasOwner", BLUEZ_SERVICE);

    if (!nameExists.isValid() || !nameExists.value()) {
        qWarning() << "BlueZ service is NOT running on D-Bus!";
        qWarning() << "Try: sudo systemctl start bluetooth";
        return {};
    }
    qDebug() << "BlueZ service is running on D-Bus";

    // Call GetManagedObjects
    QDBusMessage msg = QDBusMessage::createMethodCall(
        BLUEZ_SERVICE,
        "/",
        DBUS_OBJMGR,
        "GetManagedObjects"
        );

    QDBusMessage response = bus.call(msg);

    if (response.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetManagedObjects failed:" << response.errorMessage();
        return {};
    }

    if (response.arguments().isEmpty()) {
        qWarning() << "GetManagedObjects returned empty response";
        return {};
    }

    qDebug() << "GetManagedObjects succeeded";

    // Parse the response - structure is: a{oa{sa{sv}}}
    // Dict< ObjectPath, Dict< InterfaceName, Dict< PropertyName, Variant >>>
    const QDBusArgument &arg = response.arguments().first().value<QDBusArgument>();

    QString adapterPath;

    // Level 1: Map of ObjectPath -> Interfaces
    arg.beginMap();
    while (!arg.atEnd()) {
        QDBusObjectPath objectPath;

        arg.beginMapEntry();
        arg >> objectPath;

        qDebug() << "  Object:" << objectPath.path();

        // Level 2: Map of InterfaceName -> Properties
        arg.beginMap();
        while (!arg.atEnd()) {
            QString interfaceName;

            arg.beginMapEntry();
            arg >> interfaceName;

            // Level 3: Map of PropertyName -> Variant (we skip reading properties here)
            arg.beginMap();
            while (!arg.atEnd()) {
                QString propertyName;
                QVariant propertyValue;

                arg.beginMapEntry();
                arg >> propertyName;
                arg >> propertyValue;
                arg.endMapEntry();
            }
            arg.endMap();

            arg.endMapEntry();

            qDebug() << "    Interface:" << interfaceName;

            if (interfaceName == BLUEZ_ADAPTER) {
                qDebug() << "    *** Found Bluetooth adapter! ***";
                adapterPath = objectPath.path();
            }
        }
        arg.endMap();

        arg.endMapEntry();
    }
    arg.endMap();

    if (adapterPath.isEmpty()) {
        qWarning() << "No adapter with interface" << BLUEZ_ADAPTER << "found";
    } else {
        qDebug() << "Using adapter:" << adapterPath;
    }

    return adapterPath;
}

void BluetoothManager::loadKnownDevices()
{
    qDebug() << "=== loadKnownDevices() called ===";

    QDBusConnection bus = QDBusConnection::systemBus();

    QDBusMessage msg = QDBusMessage::createMethodCall(
        BLUEZ_SERVICE,
        "/",
        DBUS_OBJMGR,
        "GetManagedObjects"
        );

    QDBusMessage response = bus.call(msg);

    if (response.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetManagedObjects failed:" << response.errorMessage();
        return;
    }

    if (response.arguments().isEmpty()) {
        qWarning() << "Empty response from GetManagedObjects";
        return;
    }

    const QDBusArgument &arg = response.arguments().first().value<QDBusArgument>();

    // Level 1: Map of ObjectPath -> Interfaces
    arg.beginMap();
    while (!arg.atEnd()) {
        QDBusObjectPath objectPath;

        arg.beginMapEntry();
        arg >> objectPath;

        // Level 2: Map of InterfaceName -> Properties
        arg.beginMap();
        while (!arg.atEnd()) {
            QString interfaceName;
            QVariantMap props;

            arg.beginMapEntry();
            arg >> interfaceName;

            // Level 3: Map of PropertyName -> Variant
            arg.beginMap();
            while (!arg.atEnd()) {
                QString propertyName;
                QDBusVariant propertyValue;

                arg.beginMapEntry();
                arg >> propertyName;
                arg >> propertyValue;
                arg.endMapEntry();

                props.insert(propertyName, propertyValue.variant());
            }
            arg.endMap();

            arg.endMapEntry();

            // Process device interfaces
            if (interfaceName == BLUEZ_DEVICE) {
                BluetoothDevice dev;
                dev.objectPath  = objectPath.path();
                dev.name        = props.value("Name").toString();
                dev.address     = props.value("Address").toString();
                dev.isPaired    = props.value("Paired").toBool();
                dev.isConnected = props.value("Connected").toBool();
                dev.deviceClass = props.value("Class").toUInt();

                qDebug() << "  Found device:" << dev.name << "(" << dev.address << ")";

                if (!dev.address.isEmpty()) {
                    m_devices->addOrUpdateDevice(dev);
                }
            }
        }
        arg.endMap();

        arg.endMapEntry();
    }
    arg.endMap();

    emit devicesChanged();
}

void BluetoothManager::setEnabled(bool on)
{
    if (!m_adapterIface) return;

    QDBusInterface propsIface(BLUEZ_SERVICE, m_adapterPath.path(),
                              DBUS_PROP, QDBusConnection::systemBus());
    propsIface.call("Set", BLUEZ_ADAPTER, "Powered",
                    QVariant::fromValue(QDBusVariant(on)));

    m_enabled = on;
    emit enabledChanged();

    if (!on) stopDiscovery();
}

void BluetoothManager::startDiscovery()
{
    if (!m_adapterIface || !m_enabled) return;
    m_adapterIface->call("StartDiscovery");
}

void BluetoothManager::stopDiscovery()
{
    if (!m_adapterIface) return;
    m_adapterIface->call("StopDiscovery");
}

void BluetoothManager::pairDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    QDBusInterface devIface(BLUEZ_SERVICE, path, BLUEZ_DEVICE,
                            QDBusConnection::systemBus());
    QDBusReply<void> reply = devIface.call("Pair");

    if (!reply.isValid())
        emit pairingError("Pairing failed: " + reply.error().message());
}

void BluetoothManager::connectDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    QDBusInterface devIface(BLUEZ_SERVICE, path, BLUEZ_DEVICE,
                            QDBusConnection::systemBus());
    QDBusReply<void> reply = devIface.call("Connect");

    if (!reply.isValid())
        emit connectionError("Connection failed: " + reply.error().message());
}

void BluetoothManager::disconnectDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    QDBusInterface devIface(BLUEZ_SERVICE, path, BLUEZ_DEVICE,
                            QDBusConnection::systemBus());
    devIface.call("Disconnect");
}

void BluetoothManager::removeDevice(const QString &address)
{
    QString path = devicePathFromAddress(address);
    if (path.isEmpty()) return;

    m_adapterIface->call("RemoveDevice",
                         QVariant::fromValue(QDBusObjectPath(path)));
    m_devices->removeDevice(address);
}

QString BluetoothManager::devicePathFromAddress(const QString &address)
{
    QDBusInterface objMgr(BLUEZ_SERVICE, "/", DBUS_OBJMGR,
                          QDBusConnection::systemBus());
    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply =
        objMgr.call("GetManagedObjects");
    if (!reply.isValid()) return {};

    for (auto it = reply.value().begin(); it != reply.value().end(); ++it) {
        if (!it.value().contains(BLUEZ_DEVICE)) continue;
        if (it.value().value(BLUEZ_DEVICE).value("Address").toString() == address)
            return it.key().path();
    }
    return {};
}

void BluetoothManager::onInterfacesAdded(const QDBusObjectPath &path,
                                         const QVariantMap &interfaces)
{
    if (!interfaces.contains(BLUEZ_DEVICE)) return;

    const QVariantMap &props = interfaces.value(BLUEZ_DEVICE).toMap();
    BluetoothDevice dev;
    dev.objectPath  = path.path();
    dev.name        = props.value("Name").toString();
    dev.address     = props.value("Address").toString();
    dev.isPaired    = props.value("Paired").toBool();
    dev.isConnected = props.value("Connected").toBool();
    dev.deviceClass = props.value("Class").toUInt();

    if (!dev.address.isEmpty()) {
        m_devices->addOrUpdateDevice(dev);
        emit devicesChanged();
    }
}

void BluetoothManager::onInterfacesRemoved(const QDBusObjectPath &path,
                                           const QStringList &interfaces)
{
    Q_UNUSED(path)
    if (interfaces.contains(BLUEZ_DEVICE)) {
        // Find by path and remove
        // (BluetoothDeviceModel::removeDevice takes address — extend if needed)
        emit devicesChanged();
    }
}

void BluetoothManager::onPropertiesChanged(const QString &interface,
                                           const QVariantMap &changed,
                                           const QStringList &)
{
    if (interface == BLUEZ_ADAPTER) {
        if (changed.contains("Powered")) {
            m_enabled = changed.value("Powered").toBool();
            emit enabledChanged();
        }
    }
    if (interface == BLUEZ_DEVICE) {
        // A device's Connected or Paired state changed — reload
        loadKnownDevices();
    }
}
