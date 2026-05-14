import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme"

Slider {
    id: root

    implicitWidth: 200
    implicitHeight: 40

    // Fully styleable track + handle
    background: Rectangle {
        x: root.leftPadding
        y: root.topPadding + root.availableHeight / 2 - height / 2
        width:  root.availableWidth
        height: 6
        radius: 3
        color: Theme.divider

        Rectangle {
            width:  root.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color:  Theme.accent
        }
    }

    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * root.availableWidth - width / 2
        y: root.topPadding  + root.availableHeight / 2 - height / 2
        width:  22
        height: 22
        radius: 11
        color: "white"
        border.color: Theme.accent
        border.width: 2
    }
}
