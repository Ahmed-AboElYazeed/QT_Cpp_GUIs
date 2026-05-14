import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../theme"

Rectangle {
    id: root
    color: Theme.sidebarBg

    property int currentIndex: 0
    signal navigate(int index)

    // Navigation items — add new pages here only
    readonly property var items: [
        { icon: "⚙", label: "Wi-Fi"      },
        { icon: "⚙", label: "Bluetooth"  },
        { icon: "⚙", label: "Sound"      },
        { icon: "⚙", label: "Appearance" },
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // App title / header
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "Settings"
                font.pixelSize: Theme.fontSizeTitle
                font.weight: Theme.fontWeightBold
                color: Theme.textPrimary
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.divider
        }

        // Nav list
        ListView {
            id: navList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.items
            interactive: false
            spacing: 2
            topMargin: Theme.spacingSmall
            bottomMargin: Theme.spacingSmall

            delegate: SidebarItem {
                width: navList.width
                height: Theme.sidebarItemH
                icon:      modelData.icon
                label:     modelData.label
                isActive:  root.currentIndex === index
                onClicked: root.navigate(index)
            }
        }
    }
}
