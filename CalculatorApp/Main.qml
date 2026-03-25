import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 400
    height: 500
    title: qsTr("Calculator")
    visible: true

    ColumnLayout{
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle{
            radius: 15
            Layout.fillWidth: true
            height: userInputTextDisplayId.height + 30
            color: "lightgray"
            Text {
                id: userInputTextDisplayId
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 15
                text: calculator.displayText
                font.pixelSize: 35
                horizontalAlignment: Text.AlignRight
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 4
            rowSpacing: 8
            columnSpacing: 8

            // Button labels in order
            Repeater {
                model: ["C", "±", "%", "÷",
                        "1", "2", "3", "×",
                        "4", "5", "6", "-",
                        "7", "8", "9", "+",
                        "0", ".", "="]

                delegate: CalcButton {
                    text: modelData
                    // "0" spans 2 columns
                    Layout.columnSpan: modelData === "0" ? 2 : 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    // Distinguish operator buttons visually
                    isOperator: ["÷","×","-","+","="].includes(modelData)
                    isFunction: ["C","±","%"].includes(modelData)
                    // Call C++ directly from QML
                    onClicked: calculator.pressButton(modelData)
                }
            }
        }

    }

    Item {
        anchors.fill: parent
        focus: true                         // ← focus lives here, to allow this item to listen to the Qt keys

        Keys.onPressed: (event) => {
            // Map physical keys to the same labels your buttons use
            const keyMap = {
                [Qt.Key_0]:          "0",
                [Qt.Key_1]:          "1",
                [Qt.Key_2]:          "2",
                [Qt.Key_3]:          "3",
                [Qt.Key_4]:          "4",
                [Qt.Key_5]:          "5",
                [Qt.Key_6]:          "6",
                [Qt.Key_7]:          "7",
                [Qt.Key_8]:          "8",
                [Qt.Key_9]:          "9",
                [Qt.Key_Period]:     ".",
                [Qt.Key_Plus]:       "+",
                [Qt.Key_Minus]:      "-",
                [Qt.Key_Asterisk]:   "×",
                [Qt.Key_Slash]:      "÷",
                [Qt.Key_Return]:     "=",
                [Qt.Key_Enter]:      "=",    // numpad Enter
                [Qt.Key_Equal]:      "=",    // = key without Shift
                [Qt.Key_Backspace]:  "DEL",
                [Qt.Key_Escape]:     "C",
                [Qt.Key_Delete]:     "C",
            }

            const label = keyMap[event.key]
            if (label !== undefined) {
                calculator.pressButton(label)
                event.accepted = true    // stops the event bubbling further
            }
        }
    }

}
