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
            title: "Bluetooth"
            subtitle: "Pair and manage devices"
            Layout.fillWidth: true
        }

        SettingsGroup {
            Layout.fillWidth: true

            SettingsRow {
                label: "Bluetooth"
                description: "Allow this device to be discovered and connect to others"
                showDivider: false

                StyledToggle {
                    id: btToggle
                    checked: false      // ← replace with backend property
                    onToggled: (val) => console.log("Bluetooth:", val)
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "PAIRED DEVICES"
            visible: btToggle.checked

            Repeater {
                model: [
                    { name: "Galaxy Buds Pro",  connected: true  },
                    { name: "Logitech MX Keys", connected: false },
                ]

                delegate: SettingsRow {
                    label:       modelData.name
                    description: modelData.connected ? "Connected" : "Paired"
                    showDivider: index < 1

                    Rectangle {
                        width: 100
                        height: 30
                        radius: Theme.radiusSmall
                        color: modelData.connected ? Theme.danger : Theme.accent

                        Text {
                            anchors.centerIn: parent
                            text: modelData.connected ? "Disconnect" : "Connect"
                            color: Theme.textOnAccent
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: console.log("Toggle:", modelData.name)
                        }
                    }
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "NEARBY DEVICES"
            visible: btToggle.checked

            SettingsRow {
                label: "Scanning for devices..."
                showDivider: false

                Rectangle {
                    width: 80
                    height: 30
                    radius: Theme.radiusSmall
                    color: Theme.surface
                    border.color: Theme.accent
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Scan"
                        color: Theme.accent
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Start scan")
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
