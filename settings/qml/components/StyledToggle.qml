import QtQuick 2.15
import "../theme"

Rectangle {
    id: root
    property bool checked: false
    signal toggled(bool value)

    width: 48
    height: 26
    radius: height / 2
    color: checked ? Theme.accent : Theme.divider

    Behavior on color { ColorAnimation { duration: Theme.animFast } }

    Rectangle {
        id: thumb
        width: 20
        height: 20
        radius: width / 2
        color: "white"
        anchors.verticalCenter: parent.verticalCenter
        x: root.checked ? root.width - width - 3 : 3

        Behavior on x { NumberAnimation { duration: Theme.animFast } }

        layer.enabled: true
        layer.effect: null  // swap for DropShadow if Qt Graphical Effects available
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.checked = !root.checked
            root.toggled(root.checked)
        }
    }
}
