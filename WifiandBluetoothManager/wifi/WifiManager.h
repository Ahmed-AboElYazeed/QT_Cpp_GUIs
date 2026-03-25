#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include "WifiNetworkModel.h"

class WifiManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool             enabled  READ enabled  WRITE setEnabled  NOTIFY enabledChanged)
    Q_PROPERTY(WifiNetworkModel* networks READ networks                   NOTIFY networksChanged)

public:
    explicit WifiManager(QObject *parent = nullptr);

    bool             enabled()  const { return m_enabled; }
    WifiNetworkModel* networks() const { return m_networks; }

    Q_INVOKABLE void setEnabled(bool on);
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE void forgetNetwork(const QString &ssid);
    Q_INVOKABLE void disconnectCurrent();

signals:
    void enabledChanged();
    void networksChanged();
    void connectionError(const QString &message);

private slots:
    void onPropertiesChanged(const QString &interface,
                             const QVariantMap &changed,
                             const QStringList &invalidated);
    void onScanDone();

private:
    void initDBus();
    void refreshNetworks();
    QString findWirelessDevice();
    int    signalToPercent(int dBm);

    QDBusInterface*   m_nmIface     = nullptr; // org.freedesktop.NetworkManager
    QDBusInterface*   m_deviceIface = nullptr; // NM.Device.Wireless
    WifiNetworkModel* m_networks;
    bool              m_enabled = false;
    QString           m_devicePath;
};

#endif // WIFIMANAGER_H
