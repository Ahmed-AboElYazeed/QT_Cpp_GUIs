#include "wifimanager.h"
#include <QDebug>
#include <QRandomGenerator>

// ── Simulated scan data ───────────────────────────────────────────────────
// Replace this with real platform calls:
//   Linux  → NetworkManager D-Bus API  (Qt DBus)
//   Android→ QNetworkInterface / Android WiFi API via JNI
//   Windows→ WlanScan / WlanGetAvailableNetworkList (winrt::WiFiAdapter)
static QList<WifiNetwork> simulatedScan()
{
    return {
        WifiNetwork("HomeNetwork",    90, true ),
        WifiNetwork("CoffeeShop_5G",  65, true ),
        WifiNetwork("OpenNet",        40, false),
        WifiNetwork("Neighbor_2.4G",  30, true ),
        WifiNetwork("GuestNetwork",   55, false),
    };
}

// ── Constructor ───────────────────────────────────────────────────────────

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
    , m_model(new WifiNetworkModel(this))   // model owned by this manager
{
    // Scan timer — fires once after 2 s to mimic async scan completion
    m_scanTimer = new QTimer(this);
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout, this, &WifiManager::onScanFinished);

    // Connect timer — fires once after 1.5 s to mimic async connect attempt
    m_connectTimer = new QTimer(this);
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout, this, &WifiManager::onConnectFinished);
}

// ── Property setters ──────────────────────────────────────────────────────

void WifiManager::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    emit enabledChanged();

    if (m_enabled) {
        // Auto-scan when user turns Wi-Fi on
        scan();
    } else {
        // Clear the list and reset connection state when Wi-Fi is turned off
        m_model->setNetworks({});
        setConnectedSsid(QString());
        setStatusText(QString());
    }

    qDebug() << "[WifiManager] enabled =" << m_enabled;
}

void WifiManager::setScanning(bool v)
{
    if (m_scanning == v) return;
    m_scanning = v;
    emit scanningChanged();
}

void WifiManager::setConnectedSsid(const QString &ssid)
{
    if (m_connectedSsid == ssid) return;
    m_connectedSsid = ssid;
    emit connectedSsidChanged();
}

void WifiManager::setStatusText(const QString &text)
{
    if (m_statusText == text) return;
    m_statusText = text;
    emit statusTextChanged();
}

// ── Public Q_INVOKABLEs ───────────────────────────────────────────────────

void WifiManager::scan()
{
    if (!m_enabled || m_scanning) return;

    setScanning(true);
    setStatusText("Scanning…");

    // In a real implementation: start NetworkManager scan here, connect to
    // its "ScanDone" signal instead of using a timer.
    m_scanTimer->start(2000);   // 2-second simulated delay

    qDebug() << "[WifiManager] scan started";
}

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    if (!m_enabled) return;

    m_pendingSsid = ssid;
    setStatusText(QString("Connecting to %1…").arg(ssid));

    // Real implementation: call NetworkManager AddAndActivateConnection / ActivateConnection
    // Pass credentials (password) here.
    Q_UNUSED(password)  // remove when wiring to real backend

    m_connectTimer->start(1500);    // 1.5-second simulated connect delay

    qDebug() << "[WifiManager] connecting to" << ssid;
}

void WifiManager::disconnect()
{
    if (m_connectedSsid.isEmpty()) return;

    // Real implementation: NetworkManager.Disconnect()
    m_model->setConnected(m_connectedSsid, false);
    setConnectedSsid(QString());
    setStatusText("Disconnected");

    qDebug() << "[WifiManager] disconnected";
}

// ── Private slots (timer callbacks) ──────────────────────────────────────

void WifiManager::onScanFinished()
{
    // Replace simulatedScan() with the real results from your platform API
    m_model->setNetworks(simulatedScan());
    setScanning(false);
    setStatusText("Scan complete");

    qDebug() << "[WifiManager] scan complete —"
             << m_model->rowCount() << "networks found";
}

void WifiManager::onConnectFinished()
{
    // Simulate 80 % success rate for testing.
    // Real implementation: check NetworkManager activation state here.
    const bool success = QRandomGenerator::global()->bounded(10) < 8;

    if (success) {
        m_model->setConnected(m_pendingSsid, true);
        setConnectedSsid(m_pendingSsid);
        setStatusText(QString("Connected to %1").arg(m_pendingSsid));
    } else {
        setStatusText(QString("Failed to connect to %1").arg(m_pendingSsid));
    }

    // QML listens to this signal to show a toast / dialog
    emit connectionResult(success, m_statusText);

    m_pendingSsid.clear();
    qDebug() << "[WifiManager] connect result:" << success;
}
