import QtQuick 2.15

Rectangle {
    id: splash
    signal splashDone()
    color: "#1a0a00"

    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
            text: "🐴"
            font.pixelSize: 90
            anchors.horizontalCenter: parent.horizontalCenter

            SequentialAnimation on scale {
                loops: Animation.Infinite
                NumberAnimation { to: 1.1; duration: 800; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
            }
        }

        Text {
            text: "Equine World"
            color: "#f5c842"
            font.pixelSize: 38
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Your Horse Companion"
            color: "#c8a96e"
            font.pixelSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            width: 260; height: 5
            radius: 3
            color: "#3d2200"
            anchors.horizontalCenter: parent.horizontalCenter

            Rectangle {
                width: 0; height: 5
                radius: 3
                color: "#f5c842"
                NumberAnimation on width {
                    from: 0; to: 260
                    duration: 3000
                    easing.type: Easing.InOutCubic
                    running: true
                }
            }
        }
    }

    Timer {
        interval: 3000
        running: true
        onTriggered: fadeOut.start()
    }

    SequentialAnimation {
        id: fadeOut
        NumberAnimation { target: splash; property: "opacity"; to: 0; duration: 500 }
        ScriptAction { script: splash.splashDone() }
    }
}
