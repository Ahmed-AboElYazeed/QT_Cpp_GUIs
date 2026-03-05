import QtQuick 2.15
import QtQuick.Controls 2.15
import HoursesGalaryGUI

Popup {
    id: popup
    width: 460*2; height: 340*1.5
    anchors.centerIn: Overlay.overlay
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string horseName: ""
    property string horseImage: ""
    property string horseDesc: ""
    property string horseBreed: ""

    background: Rectangle {
        color: "#1a0a00"
        radius: 18
        border.color: "#f5c842"
        border.width: 2
    }

    Row {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Horse image
        Rectangle {
            width: 160*2; height: parent.height
            radius: 12
            color: "#2e1500"
            clip: true

            Image {
                anchors.fill: parent
                source: popup.horseImage
                fillMode: Image.PreserveAspectCrop
            }

            // Fallback if image missing
            Text {
                anchors.centerIn: parent
                text: "🐴"
                font.pixelSize: 60
                visible: parent.children[0].status !== Image.Ready
            }
        }

        // Info
        Column {
            width: parent.width - 400
            height: parent.height
            spacing: 12

            Text {
                text: popup.horseName
                color: "#f5c842"
                font.pixelSize: 22
                font.bold: true
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: "Breed: " + popup.horseBreed
                color: "#c8a96e"
                font.pixelSize: 14
                font.italic: true
            }

            Rectangle { width: parent.width; height: 1; color: "#f5c84250" }

            Text {
                text: popup.horseDesc
                color: "#e8d5b0"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
                lineHeight: 1.6
            }

            Item { height: 10 }

            Rectangle {
                width: 110; height: 34
                radius: 10
                color: "#f5c842"

                Text {
                    anchors.centerIn: parent
                    text: "Close"
                    color: "#1a0a00"
                    font.pixelSize: 13
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: popup.close()
                }
            }
        }
    }
}
