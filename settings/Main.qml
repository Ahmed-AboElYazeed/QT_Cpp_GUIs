import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qml/theme"
import "qml/sidebar"
import "qml/pages"

ApplicationWindow {
    id: root
    visible: true
    minimumWidth: 860
    minimumHeight: 580
    width: 980
    height: 660
    title: "Settings"

    // Theme object accessible everywhere via root.theme
    property QtObject theme: Theme

    background: Rectangle { color: Theme.background }

    // Keyboard focus catcher
    Item {
        anchors.fill: parent
        focus: true

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // ── Sidebar ───────────────────────────────────────────────
            Sidebar {
                id: sidebar
                Layout.fillHeight: true
                Layout.preferredWidth: Theme.sidebarWidth
                currentIndex: stack.currentIndex
                onNavigate: (idx) => stack.currentIndex = idx
            }

            // Thin divider line
            Rectangle {
                Layout.fillHeight: true
                width: 1
                color: Theme.divider
            }

            // ── Page area ─────────────────────────────────────────────
            StackLayout {
                id: stack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: 0

                WifiPage      {}
                BluetoothPage {}
                VolumePage    {}
                ThemePage     {}
            }
        }
    }
}
