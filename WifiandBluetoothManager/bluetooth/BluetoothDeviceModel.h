#ifndef BLUETOOTHDEVICEMODEL_H
#define BLUETOOTHDEVICEMODEL_H

#include <QAbstractListModel>
#include <QList>

struct BluetoothDevice {
    QString name;
    QString address;
    QString objectPath; // D-Bus path under /org/bluez/...
    bool    isPaired;
    bool    isConnected;
    quint32 deviceClass;
};

class BluetoothDeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        AddressRole,
        ObjectPathRole,
        IsPairedRole,
        IsConnectedRole,
        DeviceClassRole
    };

    explicit BluetoothDeviceModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrUpdateDevice(const BluetoothDevice &device);
    void removeDevice(const QString &address);
    void clear();

private:
    QList<BluetoothDevice> m_devices;
};

#endif // BLUETOOTHDEVICEMODEL_H
