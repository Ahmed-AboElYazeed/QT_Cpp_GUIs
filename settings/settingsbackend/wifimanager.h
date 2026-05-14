#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QDBusObjectPath>

struct WifiNetwork {
    QString ssid;
    int     strength  = 0;
    bool    secured   = false;
    bool    connected = false;
    QString apPath;
};

class WifiNetworkModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum NetworkRoles {
        SsidRole = Qt::UserRole + 1,
        StrengthRole,
        SecuredRole,
        ConnectedRole,
        ApPathRole
    };

    explicit WifiNetworkModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setNetworks(const QList<WifiNetwork> &networks);
    void updateConnectionState(const QString &connectedSsid);
    QList<WifiNetwork> networks() const;

private:
    QList<WifiNetwork> m_networks;
};

class WifiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool              enabled       READ enabled       WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool              scanning      READ scanning                       NOTIFY scanningChanged)
    Q_PROPERTY(QString           statusText    READ statusText                     NOTIFY statusTextChanged)
    Q_PROPERTY(QString           connectedSsid READ connectedSsid                 NOTIFY connectedSsidChanged)
    Q_PROPERTY(WifiNetworkModel* networks      READ networks      CONSTANT)

public:
    explicit WifiManager(QObject *parent = nullptr);

    bool              enabled()       const;
    bool              scanning()      const;
    QString           statusText()    const;
    QString           connectedSsid() const;
    WifiNetworkModel* networks();

    Q_INVOKABLE void setEnabled(bool enabled);
    Q_INVOKABLE void scan();
    Q_INVOKABLE void connectToNetwork(const QString &ssid,
                                      const QString &password = {});
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void forgetNetwork(const QString &ssid);

signals:
    void enabledChanged();
    void scanningChanged();
    void statusTextChanged();
    void connectedSsidChanged();
    void connectionResult(bool success, const QString &message);

private slots:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);

    // FIX 1: listens to NM global StateChanged
    void onNMStateChanged(uint newState);

    // FIX 2: listens to device-level connection state changes
    void onDeviceStateChanged(uint newState, uint oldState, uint reason);

    void refreshNetworks();

private:
    bool    readWirelessEnabled() const;     // helper: always reads from D-Bus
    QString findWifiDevicePath();
    void    initDBusMonitoring();

    bool              m_enabled     = false;
    bool              m_scanning    = false;
    QString           m_statusText;
    QString           m_connectedSsid;
    WifiNetworkModel  m_networks;
    QString           m_wifiDevicePath;
};

#endif // WIFIMANAGER_H
