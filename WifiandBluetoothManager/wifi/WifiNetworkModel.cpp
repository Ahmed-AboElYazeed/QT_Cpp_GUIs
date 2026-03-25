#include "WifiNetworkModel.h"

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent) {}

int WifiNetworkModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_networks.size();
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_networks.size())
        return {};

    const WifiNetwork &n = m_networks.at(index.row());
    switch (role) {
    case SsidRole:            return n.ssid;
    case SignalStrengthRole:  return n.signalStrength;
    case IsConnectedRole:     return n.isConnected;
    case IsSecuredRole:       return n.isSecured;
    case AccessPointPathRole: return n.accessPointPath;
    default:                  return {};
    }
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const
{
    return {
            { SsidRole,            "ssid"            },
            { SignalStrengthRole,  "signalStrength"  },
            { IsConnectedRole,     "isConnected"     },
            { IsSecuredRole,       "isSecured"       },
            { AccessPointPathRole, "accessPointPath" },
            };
}

void WifiNetworkModel::setNetworks(const QList<WifiNetwork> &networks)
{
    beginResetModel();
    m_networks = networks;
    endResetModel();
}

void WifiNetworkModel::updateConnectionStatus(const QString &ssid, bool connected)
{
    for (int i = 0; i < m_networks.size(); ++i) {
        if (m_networks[i].ssid == ssid) {
            m_networks[i].isConnected = connected;
            emit dataChanged(index(i), index(i), { IsConnectedRole });
            return;
        }
    }
}

void WifiNetworkModel::clear()
{
    beginResetModel();
    m_networks.clear();
    endResetModel();
}
