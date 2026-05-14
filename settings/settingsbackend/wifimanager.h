#pragma once

#include <QObject>
#include <QDBusInterface>

#include "settingsbackend/wifinetworkmodel.h"

class WifiManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled
                   READ enabled
                       WRITE setEnabled
                           NOTIFY enabledChanged)

    Q_PROPERTY(bool scanning
                   READ scanning
                       NOTIFY scanningChanged)

    Q_PROPERTY(QString statusText
                   READ statusText
                       NOTIFY statusTextChanged)

    Q_PROPERTY(WifiNetworkModel* networks
                   READ networks
                       CONSTANT)

public:
    explicit WifiManager(QObject *parent = nullptr);

    bool enabled() const;
    bool scanning() const;

    QString statusText() const;

    WifiNetworkModel* networks();

    void setEnabled(bool enabled);

    Q_INVOKABLE void scan();

signals:
    void enabledChanged();
    void scanningChanged();
    void statusTextChanged();

private:
    QString findWifiDevicePath();

    bool m_enabled = true;
    bool m_scanning = false;

    QString m_statusText = "Ready";

    WifiNetworkModel m_networks;
};
