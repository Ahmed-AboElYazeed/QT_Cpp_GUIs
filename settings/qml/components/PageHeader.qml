import QtQuick 2.15
import "../theme"

Item {
    property string title:    ""
    property string subtitle: ""

    height: subtitleText.visible ? 64 : 48

    Text {
        id: titleText
        text: title
        font.pixelSize: Theme.fontSizeHeading
        font.weight: Theme.fontWeightBold
        color: Theme.textPrimary
        anchors.top: parent.top
    }

    Text {
        id: subtitleText
        text: subtitle
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.textSecondary
        visible: subtitle.length > 0
        anchors.top: titleText.bottom
        anchors.topMargin: 2
    }
}
