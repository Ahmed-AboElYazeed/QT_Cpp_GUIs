#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QSet>

struct BluetoothDevice {
    QString name;
    QString address;
    QString objectPath;
    bool    paired      = false;
    bool    connected   = false;
    bool    trusted     = false;
    quint32 deviceClass = 0;

    QString typeHint() const {
        uint major = (deviceClass >> 8) & 0x1F;
        switch (major) {
        case 0x01: return "Computer";
        case 0x02: return "Phone";
        case 0x04: return "Audio / Headset";
        case 0x05: return "Peripheral";
        default:   return "Device";
        }
    }
};

class BluetoothDeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        AddressRole,
        ObjectPathRole,
        PairedRole,
        ConnectedRole,
        TrustedRole,
        TypeHintRole
    };

    explicit BluetoothDeviceModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrUpdate(const BluetoothDevice &device);
    void remove(const QString &address);
    void clear();
    QList<BluetoothDevice> devices() const;

private:
    QList<BluetoothDevice> m_devices;
};

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool                  enabled     READ enabled
                   WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool                  discovering READ discovering
                   NOTIFY discoveringChanged)
    Q_PROPERTY(QString               statusText  READ statusText
                   NOTIFY statusTextChanged)
    Q_PROPERTY(BluetoothDeviceModel* devices     READ devices CONSTANT)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    bool                  enabled()     const;
    bool                  discovering() const;
    QString               statusText()  const;
    BluetoothDeviceModel* devices();

    Q_INVOKABLE void setEnabled(bool on);
    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void pairDevice(const QString &address);
    Q_INVOKABLE void connectDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice(const QString &address);
    Q_INVOKABLE void removeDevice(const QString &address);

signals:
    void enabledChanged();
    void discoveringChanged();
    void statusTextChanged();
    void operationResult(bool success, const QString &message);

private slots:
    void onInterfacesAdded(const QDBusObjectPath &path,
                           const QVariantMap &interfaces);
    void onInterfacesRemoved(const QDBusObjectPath &path,
                             const QStringList &interfaces);
    void onAdapterPropertiesChanged(const QString &interface,
                                    const QVariantMap &changed,
                                    const QStringList &invalidated);
    void onDevicePropertiesChanged(const QString &interface,
                                   const QVariantMap &changed,
                                   const QStringList &invalidated);

private:
    void    initDBus();
    QString findAdapterPath();
    void    loadExistingDevices();
    void    subscribeToDevice(const QString &devicePath);
    BluetoothDevice deviceFromDBusProps(const QString &path,
                                        const QVariantMap &props) const;
    QString devicePathFromAddress(const QString &address) const;
    void    updateStatus();

    QString               m_adapterPath;
    bool                  m_enabled     = false;
    bool                  m_discovering = false;
    QString               m_statusText;
    BluetoothDeviceModel  m_devices;
    QSet<QString>         m_subscribedPaths;  // prevents duplicate subscriptions
};

#endif