#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>

class SystemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double cpuTemp READ cpuTemp NOTIFY cpuTempChanged)

public:
    explicit SystemInfo(QObject *parent = nullptr) : QObject(parent), m_cpuTemp(0.0)
    {
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &SystemInfo::updateTemperature);
        timer->start(3000);
        updateTemperature();
    }

    double cpuTemp() const { return m_cpuTemp; }

signals:
    void cpuTempChanged();

private slots:
    void updateTemperature()
    {
        QFile file("/sys/class/thermal/thermal_zone0/temp");
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            m_cpuTemp = stream.readLine().toInt() / 1000.0;
            file.close();
        } else {
            m_cpuTemp = 45.0 + (rand() % 20);
        }
        emit cpuTempChanged();
    }

private:
    double m_cpuTemp;
};

#endif
