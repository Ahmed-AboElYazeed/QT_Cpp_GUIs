import QtQuick 2.15

Rectangle {
    id: root
    property bool checked: false
    signal toggled()

    width: 52
    height: 28
    radius: 14
    color: checked ? "#4caf50" : "#9e9e9e"

    Rectangle {
        id: thumb
        width: 22
        height: 22
        radius: 11
        color: "white"
        anchors.verticalCenter: parent.verticalCenter
        x: checked ? parent.width - width - 3 : 3

        Behavior on x {
            NumberAnimation { duration: 150 }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.checked = !root.checked
            root.toggled()
        }
    }
}
