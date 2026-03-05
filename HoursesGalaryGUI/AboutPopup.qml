import QtQuick 2.15
import QtQuick.Controls 2.15
import HoursesGalaryGUI

Popup {
    width: 420; height: 300
    anchors.centerIn: Overlay.overlay
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: "#1a0a00"
        radius: 18
        border.color: "#f5c842"
        border.width: 2
    }

    Column {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 14

        Text {
            text: "🐴  Equine World"
            color: "#f5c842"
            font.pixelSize: 24
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle { width: parent.width; height: 1; color: "#f5c84250" }

        Text {
            text: "Version 1.0.0"
            color: "#c8a96e"
            font.pixelSize: 13
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            width: parent.width
            text: "Equine World is a dedicated app for horse lovers and breeders. " +
                  "Browse our curated gallery of horses, explore detailed profiles, " +
                  "and monitor your system while you ride through the experience."
            color: "#e8d5b0"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            lineHeight: 1.6
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            width: 110; height: 36
            radius: 10
            color: "#f5c842"
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                anchors.centerIn: parent
                text: "Close"
                color: "#1a0a00"
                font.pixelSize: 14
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: close()
            }
        }
    }
}
