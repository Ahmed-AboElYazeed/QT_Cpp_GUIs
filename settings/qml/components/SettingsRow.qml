import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../theme"

Rectangle {
    id: root

    property string label: ""
    property string description: ""
    property bool showDivider: true

    default property alias control: controlSlot.data

    width: parent ? parent.width : implicitWidth
    implicitHeight: contentLayout.implicitHeight + Theme.spacingMedium * 1.5

    color: "transparent"

    antialiasing: true
    smooth: true

    RowLayout {
        id: contentLayout

        anchors {
            fill: parent
            leftMargin: Theme.spacingMedium
            rightMargin: Theme.spacingMedium
            topMargin: Theme.spacingSmall
            bottomMargin: Theme.spacingSmall
        }

        spacing: Theme.spacingMedium

        ColumnLayout {
            Layout.fillWidth: true

            spacing: 4

            Text {
                text: root.label

                font.pixelSize: Theme.fontSizeBody
                font.weight: Font.Medium

                color: Theme.textPrimary

                Layout.fillWidth: true

                elide: Text.ElideRight
            }

            Text {
                text: root.description

                visible: text.length > 0

                font.pixelSize: Theme.fontSizeSmall

                color: Theme.textSecondary

                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }
        }

        // Right-side widget/control
        Item {
            id: controlSlot

            Layout.alignment: Qt.AlignVCenter

            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: childrenRect.height
        }
    }

    // Bottom divider
    Rectangle {
        visible: root.showDivider

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom

            leftMargin: Theme.spacingMedium
            rightMargin: Theme.spacingMedium
        }

        height: 1

        color: Theme.divider

        opacity: 0.7
    }
}
