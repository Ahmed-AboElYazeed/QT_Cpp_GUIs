import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string name:        ""
    property string address:     ""
    property bool   isPaired:    false
    property bool   isConnected: false

    signal pairClicked()
    signal connectClicked()
    signal disconnectClicked()
    signal removeClicked()

    height: 72
    radius: 10
    color: isConnected ? "#e3f2fd" : "#f5f5f5"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Column {
            Layout.fillWidth: true
            spacing: 3

            Text {
                text: name.length > 0 ? name : address
                font.pixelSize: 15
                font.weight: Font.Medium
            }
            Text {
                text: isConnected ? "Connected" : (isPaired ? "Paired" : "Not paired")
                font.pixelSize: 12
                color: isConnected ? "#1565c0" : "#757575"
            }
            Text {
                text: address
                font.pixelSize: 11
                color: "#aaa"
            }
        }

        Button {
            text: "Pair"
            flat: true
            visible: !isPaired
            onClicked: root.pairClicked()
        }

        Button {
            text: isConnected ? "Disconnect" : "Connect"
            flat: true
            visible: isPaired
            onClicked: isConnected ? root.disconnectClicked() : root.connectClicked()
        }

        Button {
            text: "Remove"
            flat: true
            visible: isPaired
            onClicked: root.removeClicked()
        }
    }
}
