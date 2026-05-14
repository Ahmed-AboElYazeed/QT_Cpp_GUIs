import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"

Rectangle {
    id: root

    property string groupTitle: ""

    default property alias content: contentColumn.data

    width: parent ? parent.width : 0
    implicitHeight: mainColumn.implicitHeight + Theme.spacingMedium * 2

    radius: Theme.radiusMedium
    color: Theme.surface

    antialiasing: true
    smooth: true

    border.width: 1
    border.color: Qt.rgba(1,1,1,0.04)

    ColumnLayout {
        id: mainColumn

        anchors.fill: parent
        anchors.margins: Theme.spacingMedium

        spacing: Theme.spacingSmall

        // Group title
        Text {
            text: root.groupTitle

            visible: text.length > 0

            font.pixelSize: Theme.fontSizeSmall
            font.weight: Theme.fontWeightMedium

            color: Theme.textSecondary

            Layout.fillWidth: true

            bottomPadding: Theme.spacingSmall
        }

        // Settings rows container
        Column {
            id: contentColumn

            Layout.fillWidth: true

            spacing: 0
        }
    }
}
