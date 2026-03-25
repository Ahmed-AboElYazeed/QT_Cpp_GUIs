import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    visible: true
    width: 500
    height: 600
    title: "Network Manager"

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        TabButton { text: "Wi-Fi" }
        TabButton { text: "Bluetooth" }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tabBar.currentIndex

        WifiPage {}
        BluetoothPage {}
    }

    // Global error popup
    Popup {
        id: errorPopup
        property string message: ""
        anchors.centerIn: parent
        width: 300
        padding: 16

        Column {
            spacing: 12
            Text {
                text: errorPopup.message
                wrapMode: Text.WordWrap
                width: 268
            }
            Button {
                text: "OK"
                onClicked: errorPopup.close()
            }
        }
    }

    Connections {
        target: app.wifi
        function onConnectionError(msg) {
            errorPopup.message = msg
            errorPopup.open()
        }
    }

    Connections {
        target: app.bluetooth
        function onConnectionError(msg) {
            errorPopup.message = msg
            errorPopup.open()
        }
        function onPairingError(msg) {
            errorPopup.message = msg
            errorPopup.open()
        }
    }
}
