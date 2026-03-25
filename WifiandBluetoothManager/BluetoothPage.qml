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
                text: "Bluetooth"
                font.pixelSize: 18
                font.weight: Font.Medium
                Layout.fillWidth: true
            }

            ToggleSwitch {
                checked: app.bluetooth.enabled
                onToggled: app.bluetooth.setEnabled(checked)
            }
        }

        // --- Discover buttons ---
        RowLayout {
            visible: app.bluetooth.enabled
            Layout.fillWidth: true

            Button {
                text: "Start scan"
                Layout.fillWidth: true
                onClicked: app.bluetooth.startDiscovery()
            }
            Button {
                text: "Stop scan"
                Layout.fillWidth: true
                onClicked: app.bluetooth.stopDiscovery()
            }
        }

        // --- Device list ---
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.bluetooth.enabled
            model: app.bluetooth.devices
            spacing: 6
            clip: true

            delegate: DeviceCard {
                width: ListView.view.width
                name:        model.name
                address:     model.address
                isPaired:    model.isPaired
                isConnected: model.isConnected

                onPairClicked:       app.bluetooth.pairDevice(address)
                onConnectClicked:    app.bluetooth.connectDevice(address)
                onDisconnectClicked: app.bluetooth.disconnectDevice(address)
                onRemoveClicked:     app.bluetooth.removeDevice(address)
            }
        }

        Label {
            text: "Bluetooth is off"
            visible: !app.bluetooth.enabled
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: "#888"
        }
    }
}
