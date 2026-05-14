#include "wifinetworkmodel.h"

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WifiNetworkModel::rowCount(const QModelIndex &) const
{
    return m_networks.count();
}

QVariant WifiNetworkModel::data(const QModelIndex &index,
                                int role) const
{
    if (!index.isValid())
        return {};

    const auto &network = m_networks[index.row()];

    switch(role)
    {
    case SsidRole:
        return network.ssid;

    case StrengthRole:
        return network.strength;
    }

    return {};
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const
{
    return {
        {SsidRole, "ssid"},
        {StrengthRole, "strength"}
    };
}

void WifiNetworkModel::setNetworks(
    const QList<WifiNetwork> &networks)
{
    beginResetModel();

    m_networks = networks;

    endResetModel();
}
