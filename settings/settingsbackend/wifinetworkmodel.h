#pragma once

#include <QAbstractListModel>

struct WifiNetwork
{
    QString ssid;
    int strength;
};

class WifiNetworkModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        SsidRole = Qt::UserRole + 1,
        StrengthRole
    };

    explicit WifiNetworkModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    void setNetworks(const QList<WifiNetwork> &networks);

private:
    QList<WifiNetwork> m_networks;
};
