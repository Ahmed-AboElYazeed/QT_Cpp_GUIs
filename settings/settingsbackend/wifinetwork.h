#pragma once
#include <QObject>
#include <QString>

/**
 * WifiNetwork
 * -----------
 * A plain data class that represents a single scanned Wi-Fi network.
 * Q_GADGET (instead of QObject) keeps it lightweight and value-copyable,
 * which is what QML ListModel / QAbstractListModel delegates expect.
 *
 * Extend this struct when you add new pages:
 *   - BluetoothPage  →  BluetoothDevice  (same pattern)
 *   - VolumePage     →  AudioDevice       (same pattern)
 */
class WifiNetwork
{
    Q_GADGET

    // Properties declared here are readable from QML delegates
    Q_PROPERTY(QString ssid     READ ssid     CONSTANT)
    Q_PROPERTY(int     strength READ strength CONSTANT)
    Q_PROPERTY(bool    secured  READ secured  CONSTANT)
    Q_PROPERTY(bool    connected READ connected CONSTANT)

public:
    WifiNetwork() = default;
    WifiNetwork(const QString &ssid, int strength, bool secured, bool connected = false)
        : m_ssid(ssid), m_strength(strength), m_secured(secured), m_connected(connected) {}

    QString ssid()      const { return m_ssid; }
    int     strength()  const { return m_strength; }
    bool    secured()   const { return m_secured; }
    bool    connected() const { return m_connected; }

    // Allow WifiManager to flip the connected flag
    void setConnected(bool v) { m_connected = v; }

private:
    QString m_ssid;
    int     m_strength  = 0;
    bool    m_secured   = false;
    bool    m_connected = false;
};

// Register so QML can receive WifiNetwork values through signals/properties
Q_DECLARE_METATYPE(WifiNetwork)
