import QtQuick 2.15
import QtQuick.Controls 2.15
import HoursesGalaryGUI

Page {
    id: homePage
    property var stack

    background: Rectangle {
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#1a0a00" }
            GradientStop { position: 1.0; color: "#2e1500" }
        }
    }

    AboutPopup { id: aboutPopup }

    // ── Header ──────────────────────────────────────────────────
    Rectangle {
        id: header
        width: parent.width
        height: 75
        color: "#1a0a00"
        border.color: "#f5c842"
        border.width: 1

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 24
            spacing: 14

            Text {
                text: "🐴"
                font.pixelSize: 36
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "Equine World"
                color: "#f5c842"
                font.pixelSize: 26
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 24
            spacing: 24

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2

                Text {
                    id: timeText
                    text: Qt.formatTime(new Date(), "hh:mm:ss")
                    color: "#f5c842"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    id: dateText
                    text: Qt.formatDate(new Date(), "dddd, MMM d yyyy")
                    color: "#c8a96e"
                    font.pixelSize: 12
                }
                Timer {
                    interval: 1000; running: true; repeat: true
                    onTriggered: {
                        timeText.text = Qt.formatTime(new Date(), "hh:mm:ss")
                        dateText.text = Qt.formatDate(new Date(), "dddd, MMM d yyyy")
                    }
                }
            }

            Rectangle {
                width: 1; height: 36
                color: "#f5c842"
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                spacing: 6
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    text: "🌡"
                    font.pixelSize: 18
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: systemInfo.cpuTemp.toFixed(1) + "°C"
                    color: systemInfo.cpuTemp > 70 ? "#ff5555" :
                           systemInfo.cpuTemp > 55 ? "#ffaa00" : "#88dd88"
                    font.pixelSize: 18
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // ── Body ────────────────────────────────────────────────────
    Column {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Item { width: 1; height: 60 }

        Text {
            text: "Welcome to Equine World 🌿"
            color: "#f5c842"
            font.pixelSize: 30
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item { width: 1; height: 16 }

        Text {
            text: "Discover, learn, and connect with the world of horses."
            color: "#c8a96e"
            font.pixelSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item { width: 1; height: 60 }

        Row {
            spacing: 80
            anchors.horizontalCenter: parent.horizontalCenter

            // About button
            Column {
                spacing: 10

                Rectangle {
                    width: 110; height: 110
                    radius: 22
                    color: infoMouse.containsMouse ? "#f5c842" : "#3d1f00"
                    border.color: "#f5c842"
                    border.width: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    Behavior on color { ColorAnimation { duration: 180 } }

                    Text {
                        anchors.centerIn: parent
                        text: "ℹ"
                        font.pixelSize: 48
                        color: infoMouse.containsMouse ? "#1a0a00" : "#f5c842"
                        Behavior on color { ColorAnimation { duration: 180 } }
                    }

                    MouseArea {
                        id: infoMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: aboutPopup.open()
                    }
                }

                Text {
                    text: "About"
                    color: "#c8a96e"
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            // Gallery button
            Column {
                spacing: 10

                Rectangle {
                    width: 110; height: 110
                    radius: 22
                    color: galleryMouse.containsMouse ? "#f5c842" : "#3d1f00"
                    border.color: "#f5c842"
                    border.width: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    Behavior on color { ColorAnimation { duration: 180 } }

                    Text {
                        anchors.centerIn: parent
                        text: "🐎"
                        font.pixelSize: 48
                    }

                    MouseArea {
                        id: galleryMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: stack.push("qrc:/qt/qml/HoursesGalaryGUI/GalleryPage.qml", { stack: stack })
                    }
                }

                Text {
                    text: "Horses"
                    color: "#c8a96e"
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
