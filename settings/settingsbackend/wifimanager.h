#pragma once
#include <QObject>
#include <QString>
#include <QTimer>
#include "settingsbackend/wifinetworkmodel.h"

/**
 * WifiManager
 * -----------
 * The C++ backend for WifiPage.qml.
 *
 * Exposed to QML via:
 *   engine.rootContext()->setContextProperty("WifiManager", &wifiManager);
 *
 * QML binds to properties and calls Q_INVOKABLE methods:
 *
 *   wifiToggle.checked  ←→  WifiManager.enabled
 *   ListView { model: WifiManager.networks }
 *   WifiManager.connectToNetwork(ssid, password)
 *   WifiManager.scan()
 *
 * ── Extending for other pages ────────────────────────────────────────────
 * BluetoothManager, VolumeManager, ThemeManager follow the exact same
 * pattern:  properties + Q_INVOKABLEs + signals.
 * ─────────────────────────────────────────────────────────────────────────
 */
class WifiManager : public QObject
{
    Q_OBJECT

    // ── Properties visible in QML ─────────────────────────────────────────

    /** Whether Wi-Fi radio is on */
    Q_PROPERTY(bool enabled
                   READ  enabled
                       WRITE setEnabled
                           NOTIFY enabledChanged)

    /** The model that feeds the ListView in WifiPage.qml */
    Q_PROPERTY(WifiNetworkModel* networks
                   READ  networks
                       CONSTANT)           // pointer never changes; contents do

    /** True while a scan is in progress (show a spinner in QML) */
    Q_PROPERTY(bool scanning
                   READ  scanning
                       NOTIFY scanningChanged)

    /** SSID of the currently connected network, "" if none */
    Q_PROPERTY(QString connectedSsid
                   READ  connectedSsid
                       NOTIFY connectedSsidChanged)

    /** Human-readable status for the UI ("Connected", "Connecting…", "Failed") */
    Q_PROPERTY(QString statusText
                   READ  statusText
                       NOTIFY statusTextChanged)

public:
    explicit WifiManager(QObject *parent = nullptr);

    // ── Property getters ──────────────────────────────────────────────────
    bool              enabled()       const { return m_enabled; }
    WifiNetworkModel* networks()      const { return m_model; }
    bool              scanning()      const { return m_scanning; }
    QString           connectedSsid() const { return m_connectedSsid; }
    QString           statusText()    const { return m_statusText; }

    // ── Property setter ───────────────────────────────────────────────────
    void setEnabled(bool enabled);

    // ── Methods callable from QML ─────────────────────────────────────────

    /** Start a network scan (populates the ListView) */
    Q_INVOKABLE void scan();

    /**
     * Connect to a network.
     * @param ssid      Network name
     * @param password  Empty string for open networks
     */
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password = QString());

    /** Disconnect from the current network */
    Q_INVOKABLE void disconnect();

signals:
    void enabledChanged();
    void scanningChanged();
    void connectedSsidChanged();
    void statusTextChanged();

    /** Emitted when a connection attempt finishes.  QML shows a toast/dialog. */
    void connectionResult(bool success, const QString &message);

private slots:
    void onScanFinished();       // called by m_scanTimer after simulated scan
    void onConnectFinished();    // called by m_connectTimer after simulated connect

private:
    void setScanning(bool v);
    void setConnectedSsid(const QString &ssid);
    void setStatusText(const QString &text);

    // Simulated async timers — replace internals with real NetworkManager / Qt WiFi calls
    QTimer *m_scanTimer    = nullptr;
    QTimer *m_connectTimer = nullptr;

    WifiNetworkModel *m_model = nullptr;

    bool    m_enabled       = false;
    bool    m_scanning      = false;
    QString m_connectedSsid;
    QString m_statusText;
    QString m_pendingSsid;      // SSID being connected to right now
};
