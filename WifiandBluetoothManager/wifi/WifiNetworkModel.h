#ifndef WIFINETWORKMODEL_H
#define WIFINETWORKMODEL_H

#include <QAbstractListModel>
#include <QList>

struct WifiNetwork {
    QString ssid;
    int     signalStrength; // 0-100
    bool    isConnected;
    bool    isSecured;
    QString accessPointPath; // D-Bus object path
};

class WifiNetworkModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        SsidRole = Qt::UserRole + 1,
        SignalStrengthRole,
        IsConnectedRole,
        IsSecuredRole,
        AccessPointPathRole
    };

    explicit WifiNetworkModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setNetworks(const QList<WifiNetwork> &networks);
    void updateConnectionStatus(const QString &ssid, bool connected);
    void clear();

private:
    QList<WifiNetwork> m_networks;
};

#endif // WIFINETWORKMODEL_H
