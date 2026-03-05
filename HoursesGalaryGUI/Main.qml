import QtQuick 2.15
import QtQuick.Controls 2.15
import HoursesGalaryGUI

ApplicationWindow {
    id: root
    width: 1024; height: 700
    visible: true
    title: "Equine World"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: HomePage {
            stack: stackView
        }
    }

    SplashScreen {
        anchors.fill: parent
        z: 999
        onSplashDone: visible = false
    }
}
