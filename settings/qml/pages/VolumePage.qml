import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "../components"

Item {
    ColumnLayout {
        anchors {
            fill: parent
            margins: Theme.spacingLarge
        }
        spacing: Theme.spacingMedium

        PageHeader {
            title: "Sound"
            subtitle: "Volume and audio output settings"
            Layout.fillWidth: true
        }

        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "OUTPUT"

            SettingsRow {
                label: "Master Volume"
                description: "Current: " + SettingsManager.sound.masterVolume + "%"

                StyledSlider {
                    id: masterSlider
                    from: 0
                    to: 100
                    value: SettingsManager.sound.masterVolume
                    width: 200

                    // Use onMoved to avoid binding loops while dragging
                    onMoved: SettingsManager.sound.setMasterVolume(Math.round(value))
                }
            }

            SettingsRow {
                label: "Mute"
                showDivider: false

                StyledToggle {
                    checked: SettingsManager.sound.mute
                    onToggled: (val) => SettingsManager.sound.setMute(val)
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "INPUT"

            SettingsRow {
                label: "Microphone Volume"
                showDivider: false

                StyledSlider {
                    from: 0
                    to: 100
                    value: SettingsManager.sound.micVolume
                    width: 200
                    onMoved: SettingsManager.sound.setMicVolume(Math.round(value))
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
