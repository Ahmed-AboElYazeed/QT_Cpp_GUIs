import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // --- Toggle row ---
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Wi-Fi"
                font.pixelSize: 18
                font.weight: Font.Medium
                Layout.fillWidth: true
            }

            ToggleSwitch {
                id: wifiToggle
                checked: app.wifi.enabled
                onToggled: app.wifi.setEnabled(checked)
            }
        }

        // --- Scan button ---
        Button {
            text: "Scan for networks"
            visible: app.wifi.enabled
            Layout.fillWidth: true
            onClicked: app.wifi.startScan()
        }

        // --- Password dialog ---
        Dialog {
            id: passwordDialog
            property string targetSsid: ""
            title: "Enter password for " + targetSsid
            anchors.centerIn: parent
            standardButtons: Dialog.Ok | Dialog.Cancel

            ColumnLayout {
                TextField {
                    id: passwordField
                    placeholderText: "Password"
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                }
            }

            onAccepted: {
                app.wifi.connectToNetwork(targetSsid, passwordField.text)
                passwordField.text = ""
            }
        }

        // --- Network list ---
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.wifi.enabled
            model: app.wifi.networks
            spacing: 6
            clip: true

            delegate: NetworkCard {
                width: ListView.view.width
                ssid:           model.ssid
                signalStrength: model.signalStrength
                isConnected:    model.isConnected
                isSecured:      model.isSecured

                onConnectClicked: {
                    if (isSecured) {
                        passwordDialog.targetSsid = ssid
                        passwordDialog.open()
                    } else {
                        app.wifi.connectToNetwork(ssid, "")
                    }
                }

                onDisconnectClicked: app.wifi.disconnectCurrent()
                onForgetClicked:     app.wifi.forgetNetwork(ssid)
            }
        }

        Label {
            text: "Wi-Fi is off"
            visible: !app.wifi.enabled
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: "#888"
        }
    }
}
