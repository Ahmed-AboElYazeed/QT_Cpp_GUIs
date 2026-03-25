#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    bool darkMode() const;
    Q_INVOKABLE void setDarkMode(bool on);
    Q_INVOKABLE void saveKnownNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE QString passwordForNetwork(const QString &ssid);

signals:
    void darkModeChanged();

private:
    QSettings m_store;
};

#endif // SETTINGSMANAGER_H
