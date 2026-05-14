import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../theme"

Rectangle {
    id: root

    property string icon:     ""
    property string label:    ""
    property bool   isActive: false

    signal clicked()

    radius: Theme.radiusSmall
    color:  isActive  ? Theme.sidebarActive :
            hov.containsMouse ? Theme.sidebarHover :
            "transparent"

    anchors.leftMargin:  Theme.spacingSmall
    anchors.rightMargin: Theme.spacingSmall

    Behavior on color {
        ColorAnimation { duration: Theme.animFast }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin:  Theme.spacingMedium
        anchors.rightMargin: Theme.spacingMedium
        spacing: Theme.spacingMedium

        Text {
            text: root.icon
            font.pixelSize: 18
            color: root.isActive ? Theme.textOnAccent : Theme.textSecondary
        }

        Text {
            text: root.label
            font.pixelSize: Theme.fontSizeBody
            font.weight: root.isActive ? Theme.fontWeightMedium : Theme.fontWeightNormal
            color: root.isActive ? Theme.textOnAccent : Theme.textPrimary
            Layout.fillWidth: true
        }
    }

    HoverHandler { id: hov }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
