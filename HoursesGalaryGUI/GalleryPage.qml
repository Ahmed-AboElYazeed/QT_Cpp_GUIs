import QtQuick 2.15
import QtQuick.Controls 2.15
import HoursesGalaryGUI

Page {
    id: galleryPage
    property var stack

    background: Rectangle {
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#1a0a00" }
            GradientStop { position: 1.0; color: "#2e1500" }
        }
    }

    property var horses: [
        { name: "Arabian Star",  breed: "Arabian",       image: "qrc:/horsesImages/h1.jpeg", desc: "A purebred Arabian known for its elegance and stamina." },
        { name: "Thunder",       breed: "Thoroughbred",  image: "qrc:/horsesImages/h2.jpeg", desc: "A powerful Thoroughbred built for speed and competition." },
        { name: "Bella",         breed: "Andalusian",    image: "qrc:/horsesImages/h3.jpeg", desc: "A graceful Andalusian mare used for dressage competitions." },
        { name: "Rocky",         breed: "Quarter Horse", image: "qrc:/horsesImages/h4.jpeg", desc: "A sturdy Quarter Horse ideal for ranch work and trail riding." },
        { name: "Shadow",        breed: "Friesian",      image: "qrc:/horsesImages/h5.jpg",  desc: "A majestic black Friesian trained for carriage driving." },
        { name: "Luna",          breed: "Appaloosa",     image: "qrc:/horsesImages/h6.jpeg", desc: "A spotted Appaloosa mare, perfect for leisure riding." },
        { name: "Blaze",         breed: "Mustang",       image: "qrc:/horsesImages/h7.jpeg", desc: "A wild Mustang with incredible endurance and free spirit." },
        { name: "Storm",         breed: "Clydesdale",    image: "qrc:/horsesImages/h8.jpeg", desc: "A gentle giant Clydesdale known for strength and calm nature." }
    ]

    ProductPopup { id: horsePopup }

    // ── Header ──────────────────────────────────────────────────
    Rectangle {
        id: header
        width: parent.width; height: 68
        color: "#1a0a00"
        border.color: "#f5c842"; border.width: 1

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            spacing: 12

            Rectangle {
                width: 38; height: 38; radius: 10
                color: backMouse.containsMouse ? "#3d1f00" : "transparent"
                border.color: "#f5c842"; border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: 150 } }

                Text {
                    anchors.centerIn: parent
                    text: "←"
                    color: "#f5c842"
                    font.pixelSize: 20
                }

                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: stack.pop()
                }
            }

            Text {
                text: "🐎  Horse Gallery"
                color: "#f5c842"
                font.pixelSize: 22
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // ── Grid ────────────────────────────────────────────────────
    ScrollView {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        clip: true

        GridView {
            width: parent.width
            cellWidth: 300; cellHeight: 270
            model: horses

            delegate: Rectangle {
                width: 280; height: 255
                radius: 14
                color: cardMouse.containsMouse ? "#3d1f00" : "#2a1000"
                border.color: cardMouse.containsMouse ? "#f5c842" : "#5a3000"
                border.width: cardMouse.containsMouse ? 2 : 1
                Behavior on color { ColorAnimation { duration: 180 } }
                Behavior on border.color { ColorAnimation { duration: 180 } }

                Column {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        width: parent.width; height: 155
                        radius: 14
                        color: "#3d1f00"
                        clip: true

                        Image {
                            id: horseImg
                            anchors.fill: parent
                            source: modelData.image
                            fillMode: Image.PreserveAspectCrop
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "🐴"
                            font.pixelSize: 55
                            visible: horseImg.status !== Image.Ready
                        }
                    }

                    Column {
                        width: parent.width
                        padding: 12
                        spacing: 3

                        Text {
                            text: modelData.name
                            color: "#f5c842"
                            font.pixelSize: 15
                            font.bold: true
                        }

                        Text {
                            text: modelData.breed
                            color: "#c8a96e"
                            font.pixelSize: 12
                            font.italic: true
                        }
                    }
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 10
                    text: "Tap to view ›"
                    color: "#f5c842"
                    font.pixelSize: 11
                    opacity: cardMouse.containsMouse ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 180 } }
                }

                MouseArea {
                    id: cardMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        horsePopup.horseName  = modelData.name
                        horsePopup.horseImage = modelData.image
                        horsePopup.horseDesc  = modelData.desc
                        horsePopup.horseBreed = modelData.breed
                        horsePopup.open()
                    }
                }
            }
        }
    }
}
