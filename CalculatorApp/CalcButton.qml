import QtQuick

Rectangle {
    id: root

    property string text: ""
    property bool isOperator: false
    property bool isFunction: false

    signal clicked()

    radius: 10
    color: isOperator ? "#f59e0b"
         : isFunction ? "#4b5563"
         : "#374151"

    Text {
        anchors.centerIn: parent
        text: root.text
        font.pixelSize: 22
        color: "white"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        // Visual press feedback
        onPressed:  root.color = Qt.darker(root.color, 1.3)
        onReleased: root.color = isOperator ? "#f59e0b"
                              : isFunction  ? "#4b5563"
                              : "#374151"
    }
}
