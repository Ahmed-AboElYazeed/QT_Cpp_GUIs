import QtQuick 2.15
import QtQuick.Layouts 1.15
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
                description: "Current: " + Math.round(masterSlider.value) + "%"

                StyledSlider {
                    id: masterSlider
                    from: 0
                    to: 100
                    value: 75
                    width: 200
                    onValueChanged: console.log("Volume:", Math.round(value))
                }
            }

            SettingsRow {
                label: "Mute"
                showDivider: false

                StyledToggle {
                    checked: false
                    onToggled: (val) => console.log("Mute:", val)
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
                    value: 60
                    width: 200
                    onValueChanged: console.log("Mic:", Math.round(value))
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
