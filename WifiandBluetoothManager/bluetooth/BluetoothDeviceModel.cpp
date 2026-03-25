#include "BluetoothDeviceModel.h"

BluetoothDeviceModel::BluetoothDeviceModel(QObject *parent)
    : QAbstractListModel(parent) {}

int BluetoothDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_devices.size();
}

QVariant BluetoothDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_devices.size())
        return {};

    const BluetoothDevice &d = m_devices.at(index.row());
    switch (role) {
    case NameRole:        return d.name;
    case AddressRole:     return d.address;
    case ObjectPathRole:  return d.objectPath;
    case IsPairedRole:    return d.isPaired;
    case IsConnectedRole: return d.isConnected;
    case DeviceClassRole: return d.deviceClass;
    default:              return {};
    }
}

QHash<int, QByteArray> BluetoothDeviceModel::roleNames() const
{
    return {
            { NameRole,        "name"        },
            { AddressRole,     "address"     },
            { ObjectPathRole,  "objectPath"  },
            { IsPairedRole,    "isPaired"    },
            { IsConnectedRole, "isConnected" },
            { DeviceClassRole, "deviceClass" },
            };
}

void BluetoothDeviceModel::addOrUpdateDevice(const BluetoothDevice &device)
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].address == device.address) {
            m_devices[i] = device;
            emit dataChanged(index(i), index(i));
            return;
        }
    }
    beginInsertRows({}, m_devices.size(), m_devices.size());
    m_devices.append(device);
    endInsertRows();
}

void BluetoothDeviceModel::removeDevice(const QString &address)
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].address == address) {
            beginRemoveRows({}, i, i);
            m_devices.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void BluetoothDeviceModel::clear()
{
    beginResetModel();
    m_devices.clear();
    endResetModel();
}
