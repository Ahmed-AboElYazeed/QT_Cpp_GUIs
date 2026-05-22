import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Window {
    id: root
    width: 900
    height: 640
    visible: true
    title: "Phone"
    minimumWidth: 400
    minimumHeight: 300

    signal closeApp()

    property int currentTab: 0
    property string dialedNumber: ""

    // ── Background ────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: Theme.bgSurface
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Top Bar (Fixed Height) ────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 70
            color: Theme.bgFooter
            border.color: Theme.b1
            border.width: 1
            z: 10

            Rectangle {
                width: 100; height: 34; radius: Theme.r1
                color: backMouse.pressed ? Qt.darker(Theme.bgHover, 1.2) : (backMouse.containsMouse ? Theme.bgHover : Theme.bgCard)
                border.color: Theme.b2; border.width: 1

                anchors.left: parent.left; anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "← Back"
                    font.pixelSize: 13; color: Theme.t0
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: backMouse
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: root.closeApp()
                }
            }

            Text {
                anchors.centerIn: parent
                text: "Phone"
                font.pixelSize: 20; font.bold: true; color: Theme.t0
            }
        }

        // ── Main Body (Split View) ────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ── Left Sidebar (Dynamic Width) ──────────────────────
            Rectangle {
                Layout.preferredWidth: Math.max(160, Math.min(260, parent.width * 0.25))
                Layout.fillHeight: true
                color: Theme.bgCard
                border.color: Theme.b1; border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    anchors.topMargin: 20
                    spacing: 8

                    Repeater {
                        model: [
                            { name: "Keypad",   icon: "🔢" },
                            { name: "Contacts", icon: "👤" },
                            { name: "Recents",  icon: "🕒" }
                        ]

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 52
                            radius: Theme.r2
                            color: root.currentTab === index ? Theme.phoneGlow : (tabMouse.containsMouse ? Theme.bgHover : "transparent")
                            border.color: root.currentTab === index ? Theme.phoneBord : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: Theme.fast } }

                            RowLayout {
                                anchors.fill: parent; anchors.margins: 12; spacing: 12
                                Text { text: modelData.icon; font.pixelSize: 18; color: root.currentTab === index ? Theme.phoneAc : Theme.t1 }
                                Text { text: modelData.name; font.pixelSize: 15; font.bold: root.currentTab === index; color: root.currentTab === index ? Theme.phoneAc : Theme.t1; Layout.fillWidth: true }
                            }

                            MouseArea {
                                id: tabMouse
                                anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: root.currentTab = index
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            // ── Right Content Area ────────────────────────────────
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentTab

                // ══════════════════════════════════════════════════
                // 1. KEYPAD VIEW
                // ══════════════════════════════════════════════════
                Item {
                    id: keypadArea

                    property real keySize: Math.max(40, Math.min(86, keypadArea.height / 8.5))
                    property real spacingSize: keySize * 0.25

                    ColumnLayout {
                        anchors.centerIn: parent
                        width: Math.min(parent.width * 0.8, 400)
                        spacing: keypadArea.spacingSize * 2

                        // ── Dialed Number Display ──
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: keypadArea.keySize

                            Text {
                                Layout.fillWidth: true
                                text: root.dialedNumber.length > 0 ? root.dialedNumber : "Enter Number"
                                font.pixelSize: root.dialedNumber.length > 0 ? keypadArea.keySize * 0.5 : keypadArea.keySize * 0.35
                                font.bold: true
                                color: root.dialedNumber.length > 0 ? Theme.t0 : Theme.t2
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideLeft
                            }

                            Rectangle {
                                width: keypadArea.keySize * 0.7; height: width; radius: Theme.rFull
                                color: bsMouse.pressed ? Theme.bgHover : "transparent"
                                visible: root.dialedNumber.length > 0
                                Text { anchors.centerIn: parent; text: "⌫"; font.pixelSize: parent.width * 0.4; color: Theme.t1 }
                                MouseArea {
                                    id: bsMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: root.dialedNumber = root.dialedNumber.slice(0, -1)
                                    onPressAndHold: root.dialedNumber = ""
                                }
                            }
                        }

                        // ── Number Pad Grid ──
                        GridLayout {
                            Layout.alignment: Qt.AlignHCenter
                            columns: 3
                            rowSpacing: keypadArea.spacingSize
                            columnSpacing: keypadArea.spacingSize * 1.5

                            Repeater {
                                model: [
                                    { n: "1", l: "" },     { n: "2", l: "ABC" },  { n: "3", l: "DEF" },
                                    { n: "4", l: "GHI" },  { n: "5", l: "JKL" },  { n: "6", l: "MNO" },
                                    { n: "7", l: "PQRS" }, { n: "8", l: "TUV" },  { n: "9", l: "WXYZ" },
                                    { n: "*", l: "" },     { n: "0", l: "+" },    { n: "#", l: "" }
                                ]

                                Rectangle {
                                    Layout.preferredWidth: keypadArea.keySize
                                    Layout.preferredHeight: keypadArea.keySize
                                    radius: Theme.rFull
                                    color: keyMouse.pressed ? Theme.bgHover : Theme.bgCard
                                    border.color: Theme.b1; border.width: 1

                                    Column {
                                        anchors.centerIn: parent
                                        spacing: -2
                                        Text { text: modelData.n; font.pixelSize: keypadArea.keySize * 0.35; font.bold: true; color: Theme.t0; anchors.horizontalCenter: parent.horizontalCenter }
                                        Text { text: modelData.l; font.pixelSize: keypadArea.keySize * 0.12; color: Theme.t1; visible: modelData.l !== ""; anchors.horizontalCenter: parent.horizontalCenter }
                                    }

                                    MouseArea {
                                        id: keyMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                        onClicked: root.dialedNumber += modelData.n
                                    }
                                }
                            }
                        }

                        // ── Call Button ──
                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: keypadArea.spacingSize
                            width: keypadArea.keySize * 1.15
                            height: width
                            radius: Theme.rFull
                            color: callMouse.pressed ? Qt.darker(Theme.phoneAc, 1.2) : Theme.phoneAc
                            border.color: Theme.phoneBord; border.width: 1

                            Rectangle {
                                anchors.fill: parent; radius: Theme.rFull; color: "transparent"; border.color: Theme.phoneGlow; border.width: 3
                            }
                            Text { text: "📞"; font.pixelSize: parent.width * 0.4; anchors.centerIn: parent }

                            MouseArea {
                                id: callMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: console.log("Dialing: " + root.dialedNumber)
                            }
                        }
                    }
                }

                // ══════════════════════════════════════════════════
                // 2. CONTACTS VIEW
                // ══════════════════════════════════════════════════
                Item {
                    ListView {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 12
                        clip: true

                        model: ListModel {
                            ListElement { name: "Alice Freeman";  number: "+1 (555) 123-4567"; tag: "Mobile" }
                            ListElement { name: "Bob Smith";      number: "+1 (555) 987-6543"; tag: "Work" }
                            ListElement { name: "Charlie Tesla";  number: "+1 (555) 555-5555"; tag: "Mobile" }
                            ListElement { name: "David Johnson";  number: "+1 (555) 111-2222"; tag: "Home" }
                            ListElement { name: "Eve Anderson";   number: "+1 (555) 999-8888"; tag: "Work" }
                        }

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 74
                            radius: Theme.r2
                            color: contactMouse.pressed ? Theme.bgHover : Theme.bgCard
                            border.color: Theme.b1; border.width: 1

                            RowLayout {
                                anchors.fill: parent; anchors.margins: 16; spacing: 16

                                Rectangle {
                                    width: 44; height: 44; radius: Theme.rFull
                                    color: Theme.bgDeep; border.color: Theme.b1; border.width: 1
                                    Text { anchors.centerIn: parent; text: model.name.charAt(0); font.pixelSize: 18; font.bold: true; color: Theme.t0 }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Text { text: model.name; color: Theme.t0; font.pixelSize: 16; font.bold: true }
                                    Text { text: model.tag + " • " + model.number; color: Theme.t1; font.pixelSize: 13 }
                                }

                                Rectangle {
                                    width: 44; height: 44; radius: Theme.rFull
                                    color: Theme.phoneGlow; border.color: Theme.phoneBord; border.width: 1
                                    Text { anchors.centerIn: parent; text: "📞"; font.pixelSize: 18; color: Theme.phoneAc }
                                }
                            }

                            MouseArea {
                                id: contactMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.dialedNumber = model.number
                                    root.currentTab = 0
                                }
                            }
                        }
                    }
                }

                // ══════════════════════════════════════════════════
                // 3. RECENTS
                // ══════════════════════════════════════════════════
                Item {
                    Text { anchors.centerIn: parent; text: "No recent calls."; color: Theme.t1; font.pixelSize: 18 }
                }
            }
        }
    }
}