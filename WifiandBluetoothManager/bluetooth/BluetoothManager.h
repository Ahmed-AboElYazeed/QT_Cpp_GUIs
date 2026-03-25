#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include "BluetoothDeviceModel.h"

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool                 enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(BluetoothDeviceModel* devices READ devices                  NOTIFY devicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    bool                 enabled() const { return m_enabled; }
    BluetoothDeviceModel* devices() const { return m_devices; }

    Q_INVOKABLE void setEnabled(bool on);
    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void pairDevice(const QString &address);
    Q_INVOKABLE void connectDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice(const QString &address);
    Q_INVOKABLE void removeDevice(const QString &address);

signals:
    void enabledChanged();
    void devicesChanged();
    void pairingError(const QString &message);
    void connectionError(const QString &message);

private slots:
    void onInterfacesAdded(const QDBusObjectPath &path,
                           const QVariantMap &interfaces);
    void onInterfacesRemoved(const QDBusObjectPath &path,
                             const QStringList &interfaces);
    void onPropertiesChanged(const QString &interface,
                             const QVariantMap &changed,
                             const QStringList &invalidated);

private:
    void initDBus();
    QString findAdapter();
    void    loadKnownDevices();
    QString devicePathFromAddress(const QString &address);

    QDBusInterface*      m_adapterIface = nullptr;
    QDBusObjectPath      m_adapterPath;
    BluetoothDeviceModel* m_devices;
    bool                 m_enabled = false;
};

#endif // BLUETOOTHMANAGER_H
