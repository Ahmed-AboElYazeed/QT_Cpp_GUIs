#pragma once
#include <QAbstractListModel>
#include <QList>
#include "settingsbackend/wifinetwork.h"

/**
 * WifiNetworkModel
 * ----------------
 * A QAbstractListModel that holds the list of scanned Wi-Fi networks.
 * Expose this to QML as a context property and bind it to a ListView.
 *
 * Roles map to the property names you use in QML delegates:
 *   model.ssid / model.strength / model.secured / model.connected
 *
 * Pattern reused for:
 *   BluetoothPage  →  BluetoothDeviceModel
 *   VolumePage     →  AudioDeviceModel
 */
class WifiNetworkModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // QML role names — must match what delegates use
    enum Roles {
        SsidRole      = Qt::UserRole + 1,
        StrengthRole,
        SecuredRole,
        ConnectedRole
    };

    explicit WifiNetworkModel(QObject *parent = nullptr);

    // ── QAbstractListModel interface ──────────────────────────────────────
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ── Mutation helpers (called by WifiManager) ──────────────────────────
    void setNetworks(const QList<WifiNetwork> &networks);   // replace whole list (after scan)
    void setConnected(const QString &ssid, bool connected); // update a single row

private:
    QList<WifiNetwork> m_networks;
};
