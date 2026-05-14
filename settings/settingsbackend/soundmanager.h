#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QTimer>
#include <alsa/asoundlib.h>

class SoundManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(int micVolume READ micVolume WRITE setMicVolume NOTIFY micVolumeChanged)

public:
    explicit SoundManager(QObject *parent = nullptr);
    ~SoundManager();

    int masterVolume() const;
    bool mute() const;
    int micVolume() const;

public slots:
    void setMasterVolume(int volume);
    void setMute(bool muted);
    void setMicVolume(int volume);

signals:
    void masterVolumeChanged();
    void muteChanged();
    void micVolumeChanged();

private slots:
    void sync();

private:
    void openMixer();
    void closeMixer();
    long volumeToAlsa(int percent, long min, long max) const;
    int alsaToVolume(long value, long min, long max) const;

    QTimer *m_syncTimer = nullptr;
    snd_mixer_t *m_mixer = nullptr;
    snd_mixer_elem_t *m_masterElem = nullptr;
    snd_mixer_elem_t *m_micElem = nullptr;

    int m_masterVolume = 0;
    bool m_mute = false;
    int m_micVolume = 0;
};

#endif // SOUNDMANAGER_H
