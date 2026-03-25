import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string ssid:           ""
    property int    signalStrength: 0
    property bool   isConnected:    false
    property bool   isSecured:      false

    signal connectClicked()
    signal disconnectClicked()
    signal forgetClicked()

    height: 68
    radius: 10
    color: isConnected ? "#e8f5e9" : "#f5f5f5"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Column {
            Layout.fillWidth: true
            spacing: 3

            Text {
                text: ssid
                font.pixelSize: 15
                font.weight: Font.Medium
            }
            Text {
                text: isConnected ? "Connected" : (isSecured ? "Secured" : "Open")
                font.pixelSize: 12
                color: isConnected ? "#388e3c" : "#757575"
            }
        }

        Text {
            text: signalStrength + "%"
            font.pixelSize: 12
            color: "#555"
        }

        Button {
            text: isConnected ? "Disconnect" : "Connect"
            flat: true
            onClicked: isConnected ? root.disconnectClicked() : root.connectClicked()
        }

        Button {
            text: "Forget"
            flat: true
            visible: isConnected
            onClicked: root.forgetClicked()
        }
    }
}
