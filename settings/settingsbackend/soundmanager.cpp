#include "soundmanager.h"
#include <QDebug>

SoundManager::SoundManager(QObject *parent)
    : QObject(parent)
{
    openMixer();
    sync();

    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(500);
    connect(m_syncTimer, &QTimer::timeout, this, &SoundManager::sync);
    m_syncTimer->start();
}

SoundManager::~SoundManager()
{
    closeMixer();
}

void SoundManager::openMixer()
{
    if (snd_mixer_open(&m_mixer, 0) < 0) {
        qWarning() << "[SoundManager] Failed to open ALSA mixer";
        return;
    }
    if (snd_mixer_attach(m_mixer, "default") < 0) {
        qWarning() << "[SoundManager] Failed to attach default card";
        closeMixer();
        return;
    }
    if (snd_mixer_selem_register(m_mixer, nullptr, nullptr) < 0) {
        closeMixer();
        return;
    }
    if (snd_mixer_load(m_mixer) < 0) {
        qWarning() << "[SoundManager] Failed to load mixer elements";
        closeMixer();
        return;
    }

    // Locate Master and Capture (or Mic) controls
    for (snd_mixer_elem_t *elem = snd_mixer_first_elem(m_mixer);
         elem; elem = snd_mixer_elem_next(elem)) {

        if (!snd_mixer_selem_is_active(elem))
            continue;

        const char *name = snd_mixer_selem_get_name(elem);

        if (!m_masterElem && strcmp(name, "Master") == 0)
            m_masterElem = elem;
        else if (!m_masterElem && strcmp(name, "PCM") == 0)
            m_masterElem = elem;

        if (!m_micElem && strcmp(name, "Capture") == 0)
            m_micElem = elem;
        else if (!m_micElem && strcmp(name, "Mic") == 0)
            m_micElem = elem;
    }
}

void SoundManager::closeMixer()
{
    if (m_mixer) {
        snd_mixer_close(m_mixer);
        m_mixer = nullptr;
    }
    m_masterElem = nullptr;
    m_micElem = nullptr;
}

long SoundManager::volumeToAlsa(int percent, long min, long max) const
{
    if (max <= min) return min;
    return min + (static_cast<long>(percent) * (max - min)) / 100;
}

int SoundManager::alsaToVolume(long value, long min, long max) const
{
    if (max <= min) return 0;
    return static_cast<int>((value - min) * 100 / (max - min));
}

int SoundManager::masterVolume() const  { return m_masterVolume; }
bool SoundManager::mute() const          { return m_mute; }
int SoundManager::micVolume() const     { return m_micVolume; }

void SoundManager::setMasterVolume(int volume)
{
    volume = qBound(0, volume, 100);
    if (!m_masterElem || !snd_mixer_selem_has_playback_volume(m_masterElem))
        return;

    long min, max;
    snd_mixer_selem_get_playback_volume_range(m_masterElem, &min, &max);
    long val = volumeToAlsa(volume, min, max);
    snd_mixer_selem_set_playback_volume_all(m_masterElem, val);

    if (m_masterVolume != volume) {
        m_masterVolume = volume;
        emit masterVolumeChanged();
    }
}

void SoundManager::setMute(bool muted)
{
    if (!m_masterElem || !snd_mixer_selem_has_playback_switch(m_masterElem))
        return;

    snd_mixer_selem_set_playback_switch_all(m_masterElem, muted ? 0 : 1);

    if (m_mute != muted) {
        m_mute = muted;
        emit muteChanged();
    }
}

void SoundManager::setMicVolume(int volume)
{
    volume = qBound(0, volume, 100);
    if (!m_micElem || !snd_mixer_selem_has_capture_volume(m_micElem))
        return;

    long min, max;
    snd_mixer_selem_get_capture_volume_range(m_micElem, &min, &max);
    long val = volumeToAlsa(volume, min, max);
    snd_mixer_selem_set_capture_volume_all(m_micElem, val);

    if (m_micVolume != volume) {
        m_micVolume = volume;
        emit micVolumeChanged();
    }
}

void SoundManager::sync()
{
    if (!m_mixer) {
        openMixer();
        if (!m_mixer) return;
    }

    // --- Master Volume ---
    if (m_masterElem && snd_mixer_selem_has_playback_volume(m_masterElem)) {
        long min, max;
        snd_mixer_selem_get_playback_volume_range(m_masterElem, &min, &max);

        long val;
        if (snd_mixer_selem_get_playback_volume(m_masterElem, SND_MIXER_SCHN_FRONT_LEFT, &val) >= 0) {
            int pct = alsaToVolume(val, min, max);
            if (m_masterVolume != pct) {
                m_masterVolume = pct;
                emit masterVolumeChanged();
            }
        }
    }

    // --- Mute ---
    if (m_masterElem && snd_mixer_selem_has_playback_switch(m_masterElem)) {
        int sw;
        if (snd_mixer_selem_get_playback_switch(m_masterElem, SND_MIXER_SCHN_FRONT_LEFT, &sw) >= 0) {
            bool muted = (sw == 0);
            if (m_mute != muted) {
                m_mute = muted;
                emit muteChanged();
            }
        }
    }

    // --- Mic Volume ---
    if (m_micElem && snd_mixer_selem_has_capture_volume(m_micElem)) {
        long min, max;
        snd_mixer_selem_get_capture_volume_range(m_micElem, &min, &max);

        long val;
        if (snd_mixer_selem_get_capture_volume(m_micElem, SND_MIXER_SCHN_FRONT_LEFT, &val) >= 0) {
            int pct = alsaToVolume(val, min, max);
            if (m_micVolume != pct) {
                m_micVolume = pct;
                emit micVolumeChanged();
            }
        }
    }
}
