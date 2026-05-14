import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "../components"

Item {
    id: wifiPage

    Component.onCompleted: {
        SettingsManager.wifi.scan()
    }

    // ---------------------------------------------------------------------
    // Toast notification
    // ---------------------------------------------------------------------
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
        color: toastSuccess ? "#2e7d32" : "#c62828"
        visible: false
        z: 100

        property bool toastSuccess: true

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

    function showToast(success, message) {
        toastBar.toastSuccess = success
        toastText.text = message
        toastBar.visible = true
        toastTimer.restart()
    }

    // ---------------------------------------------------------------------
    // Password dialog for secured networks
    // ---------------------------------------------------------------------
    Dialog {
        id: passwordDialog
        title: "Wi-Fi Password"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 320

        property string targetSsid: ""

        onAccepted: {
            if (passwordField.text.length > 0) {
                SettingsManager.wifi.connectToNetwork(targetSsid, passwordField.text)
                passwordField.text = ""
            }
        }

        onRejected: {
            passwordField.text = ""
        }

        ColumnLayout {
            spacing: Theme.spacingMedium
            width: parent.width

            Text {
                text: "Enter password for \"" + passwordDialog.targetSsid + "\""
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.textPrimary
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: "Password"
                focus: true
            }
        }
    }

    Connections {
        target: SettingsManager.wifi

        function onConnectionResult(success, message) {
            showToast(success, message)
        }
    }

    // ---------------------------------------------------------------------
    // Main layout
    // ---------------------------------------------------------------------
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingLarge
        spacing: Theme.spacingMedium

        PageHeader {
            Layout.fillWidth: true
            title: "Wi-Fi"
            subtitle: "Manage wireless networks"
        }

        // WiFi enable switch
        SettingsGroup {
            Layout.fillWidth: true

            SettingsRow {
                label: "Wi-Fi"
                description: "Turn wireless networking on or off"
                showDivider: false

                StyledToggle {
                    id: wifiToggle
                    checked: SettingsManager.wifi.enabled

                    onToggled: (value) => {
                        SettingsManager.wifi.enabled = value
                    }
                }
            }
        }

        // Networks list
        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "AVAILABLE NETWORKS"
            visible: wifiToggle.checked

            ColumnLayout {
                width: parent.width
                spacing: Theme.spacingMedium

                // Header: status + spinner + scan button
                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true
                        text: SettingsManager.wifi.statusText
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    BusyIndicator {
                        running: SettingsManager.wifi.scanning
                        visible: running
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                    }

                    Button {
                        text: "Scan"
                        onClicked: SettingsManager.wifi.scan()
                    }
                }

                // Network list
                ListView {
                    id: networkList
                    Layout.fillWidth: true
                    Layout.preferredHeight: count > 0 ? Math.min(contentHeight, 400) : 0
                    clip: true
                    model: SettingsManager.wifi.networks

                    delegate: SettingsRow {
                        width: networkList.width

                        label: model.ssid
                        description: (model.connected ? "✓ Connected · " : "")
                                   + (model.secured ? "Secured · " : "Open · ")
                                   + model.strength + "% signal"
                        showDivider: index < networkList.count - 1

                        // Action buttons anchored to the right
                        Row {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: Theme.spacingMedium
                            spacing: Theme.spacingSmall

                            // Forget (only when currently connected)
                            Rectangle {
                                visible: model.connected
                                width: 60
                                height: 28
                                radius: Theme.radiusSmall
                                color: Theme.surfaceVariant

                                Text {
                                    anchors.centerIn: parent
                                    text: "Forget"
                                    color: Theme.textPrimary
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: SettingsManager.wifi.forgetNetwork(model.ssid)
                                }
                            }

                            // Connect / Disconnect
                            Rectangle {
                                width: model.connected ? 90 : 80
                                height: 30
                                radius: Theme.radiusSmall
                                color: model.connected ? Theme.surfaceVariant : Theme.accent

                                Text {
                                    anchors.centerIn: parent
                                    text: model.connected ? "Disconnect" : "Connect"
                                    color: model.connected ? Theme.textPrimary : Theme.textOnAccent
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (model.connected) {
                                            SettingsManager.wifi.disconnect()
                                        } else if (model.secured) {
                                            passwordDialog.targetSsid = model.ssid
                                            passwordDialog.open()
                                        } else {
                                            SettingsManager.wifi.connectToNetwork(model.ssid)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Empty state
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    visible: networkList.count === 0 && !SettingsManager.wifi.scanning
                    text: "No networks found. Tap Scan to search."
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    topPadding: Theme.spacingMedium
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
