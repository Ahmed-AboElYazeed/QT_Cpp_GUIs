import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "../components"

Item {

    Connections {
        target: SettingsManager.wifi

        function onConnectionResult(success, message) {
            toastText.text = message
            toastBar.visible = true
            toastTimer.restart()
        }
    }

    Rectangle {
        id: toastBar

        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: Theme.spacingLarge
        }

        width: toastText.implicitWidth + Theme.spacingLarge * 2
        height: 36

        radius: Theme.radiusSmall

        color: success ? "#2e7d32" : "#c62828"

        visible: false
        z: 100

        property bool success: true

        Text {
            id: toastText
            anchors.centerIn: parent
            color: Theme.textOnAccent
            font.pixelSize: Theme.fontSizeSmall
        }

        Timer {
            id: toastTimer
            interval: 3000
            onTriggered: toastBar.visible = false
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingLarge

        spacing: Theme.spacingMedium

        PageHeader {
            Layout.fillWidth: true

            title: "Wi-Fi"
            subtitle: "Manage wireless networks"
        }

        SettingsGroup {
            Layout.fillWidth: true

            SettingsRow {
                label: "Wi-Fi"
                description: "Turn wireless networking on or off"
                showDivider: false

                StyledToggle {
                    id: wifiToggle

                    checked: SettingsManager.wifi.enabled

                    onToggled: (val) => {
                        SettingsManager.wifi.enabled = val
                    }
                }
            }
        }

        SettingsGroup {
            Layout.fillWidth: true

            groupTitle: "AVAILABLE NETWORKS"

            visible: wifiToggle.checked

            RowLayout {
                Layout.fillWidth: true

                spacing: Theme.spacingSmall

                Text {
                    Layout.fillWidth: true

                    text: SettingsManager.wifi.statusText

                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                }

                BusyIndicator {
                    running: SettingsManager.wifi.scanning
                    visible: running

                    width: 20
                    height: 20
                }

                Rectangle {
                    width: 60
                    height: 28

                    radius: Theme.radiusSmall

                    color: Theme.surfaceVariant

                    Text {
                        anchors.centerIn: parent

                        text: "Scan"

                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: SettingsManager.wifi.scan()
                    }
                }
            }

            ListView {
                id: networkList

                Layout.fillWidth: true

                height: Math.min(contentHeight, 400)

                clip: true

                model: SettingsManager.wifi.networks

                delegate: SettingsRow {

                    label: model.ssid

                    description:
                          (model.connected ? "✓ Connected · " : "")
                        + (model.secured ? "Secured · " : "Open · ")
                        + model.strength + "%"

                    showDivider: index < networkList.count - 1

                    Rectangle {

                        width: model.connected ? 100 : 80
                        height: 30

                        radius: Theme.radiusSmall

                        color: model.connected
                               ? Theme.surfaceVariant
                               : Theme.accent

                        Text {
                            anchors.centerIn: parent

                            text: model.connected
                                  ? "Disconnect"
                                  : "Connect"

                            color: model.connected
                                   ? Theme.textPrimary
                                   : Theme.textOnAccent

                            font.pixelSize: Theme.fontSizeSmall
                        }

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {

                                if (model.connected) {
                                    SettingsManager.wifi.disconnect()
                                }
                                else if (model.secured) {
                                    SettingsManager.wifi.connectToNetwork(
                                                model.ssid,
                                                ""
                                    )
                                }
                                else {
                                    SettingsManager.wifi.connectToNetwork(
                                                model.ssid
                                    )
                                }
                            }
                        }
                    }
                }
            }

            Text {
                Layout.fillWidth: true

                visible:
                    networkList.count === 0
                    && !SettingsManager.wifi.scanning
                    && wifiToggle.checked

                horizontalAlignment: Text.AlignHCenter

                text: "No networks found. Tap Scan to search."

                color: Theme.textSecondary

                font.pixelSize: Theme.fontSizeSmall

                topPadding: Theme.spacingMedium
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}



// import QtQuick 2.15
// import QtQuick.Layouts 1.15
// import "../theme"
// import "../components"

// Item {
//     ColumnLayout {
//         anchors {
//             fill: parent
//             margins: Theme.spacingLarge
//         }
//         spacing: Theme.spacingMedium

//         PageHeader {
//             title: "Wi-Fi"
//             subtitle: "Manage wireless networks"
//             Layout.fillWidth: true
//         }

//         // Enable / disable group
//         SettingsGroup {
//             Layout.fillWidth: true

//             SettingsRow {
//                 label: "Wi-Fi"
//                 description: "Turn wireless networking on or off"
//                 showDivider: false

//                 StyledToggle {
//                     id: wifiToggle
//                     checked: false       // ← replace with backend property
//                     onToggled: (val) => console.log("WiFi:", val)
//                 }
//             }
//         }

//         // Network list group — only visible when enabled
//         SettingsGroup {
//             Layout.fillWidth: true
//             groupTitle: "AVAILABLE NETWORKS"
//             visible: wifiToggle.checked

//             // Placeholder rows — replace with ListView + model from backend
//             Repeater {
//                 model: [
//                     { ssid: "HomeNetwork",    strength: 90, secured: true  },
//                     { ssid: "CoffeeShop_5G",  strength: 65, secured: true  },
//                     { ssid: "OpenNet",        strength: 40, secured: false  },
//                 ]

//                 delegate: SettingsRow {
//                     label:       modelData.ssid
//                     description: modelData.secured ? "Secured · " + modelData.strength + "%" : "Open · " + modelData.strength + "%"
//                     showDivider: index < 2

//                     // Connect button placeholder
//                     Rectangle {
//                         width: 80
//                         height: 30
//                         radius: Theme.radiusSmall
//                         color: Theme.accent

//                         Text {
//                             anchors.centerIn: parent
//                             text: "Connect"
//                             color: Theme.textOnAccent
//                             font.pixelSize: Theme.fontSizeSmall
//                         }

//                         MouseArea {
//                             anchors.fill: parent
//                             onClicked: console.log("Connect to", modelData.ssid)
//                         }
//                     }
//                 }
//             }
//         }

//         Item { Layout.fillHeight: true }  // pushes content up
//     }
// }
