#include "wifinetworkmodel.h"

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{}

// ── QAbstractListModel interface ──────────────────────────────────────────

int WifiNetworkModel::rowCount(const QModelIndex &parent) const
{
    // Flat list — no tree, so parent must be invalid
    if (parent.isValid()) return 0;
    return m_networks.size();
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_networks.size())
        return {};

    const WifiNetwork &net = m_networks.at(index.row());

    switch (role) {
    case SsidRole:      return net.ssid();
    case StrengthRole:  return net.strength();
    case SecuredRole:   return net.secured();
    case ConnectedRole: return net.connected();
    default:            return {};
    }
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const
{
    // These string names are what QML delegates use: model.ssid, model.strength …
    return {
            { SsidRole,      "ssid"      },
            { StrengthRole,  "strength"  },
            { SecuredRole,   "secured"   },
            { ConnectedRole, "connected" },
            };
}

// ── Mutation helpers ──────────────────────────────────────────────────────

void WifiNetworkModel::setNetworks(const QList<WifiNetwork> &networks)
{
    // beginResetModel / endResetModel tells all connected QML views to reload
    beginResetModel();
    m_networks = networks;
    endResetModel();
}

void WifiNetworkModel::setConnected(const QString &ssid, bool connected)
{
    for (int i = 0; i < m_networks.size(); ++i) {
        if (m_networks[i].ssid() == ssid) {
            m_networks[i].setConnected(connected);
            // Notify QML that just this row changed (efficient — no full reload)
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, { ConnectedRole });
        } else if (connected) {
            // Only one network can be connected at a time — disconnect the rest
            m_networks[i].setConnected(false);
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, { ConnectedRole });
        }
    }
}
