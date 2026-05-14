import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"
import "../components"

Item {
    id: btPage

    // ── Toast notification ────────────────────────────────────────────────────
    Rectangle {
        id: toastBar
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: Theme.spacingLarge
        }
        width:   toastText.implicitWidth + Theme.spacingLarge * 2
        height:  36
        radius:  Theme.radiusSmall
        color:   toastBar.toastSuccess ? "#2e7d32" : "#c62828"
        visible: false
        z:       100

        property bool toastSuccess: true

        Text {
            id: toastText
            anchors.centerIn: parent
            color:            Theme.textOnAccent
            font.pixelSize:   Theme.fontSizeSmall
        }

        Timer {
            id:       toastTimer
            interval: 3000
            onTriggered: toastBar.visible = false
        }
    }

    function showToast(success, message) {
        toastBar.toastSuccess = success
        toastText.text        = message
        toastBar.visible      = true
        toastTimer.restart()
    }

    // ── Listen for backend results ────────────────────────────────────────────
    Connections {
        target: SettingsManager.bluetooth
        function onOperationResult(success, message) {
            showToast(success, message)
        }
    }

    // ── Main layout ───────────────────────────────────────────────────────────
    ColumnLayout {
        anchors {
            fill:    parent
            margins: Theme.spacingLarge
        }
        spacing: Theme.spacingMedium

        // ── Header ───────────────────────────────────────────────────────────
        PageHeader {
            Layout.fillWidth: true
            title:    "Bluetooth"
            subtitle: "Pair and manage devices"
        }

        // ── Enable / disable ──────────────────────────────────────────────────
        SettingsGroup {
            Layout.fillWidth: true

            SettingsRow {
                label:       "Bluetooth"
                description: SettingsManager.bluetooth.statusText
                showDivider: false

                StyledToggle {
                    id:      btToggle
                    checked: SettingsManager.bluetooth.enabled
                    onToggled: (val) => SettingsManager.bluetooth.setEnabled(val)
                }
            }
        }

        // ── Everything below only shown when BT is on ─────────────────────────
        // ── Paired / connected devices ────────────────────────────────────────
        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "PAIRED DEVICES"

            // hide the whole card when BT is off or no paired devices
            visible: btToggle.checked && pairedList.count > 0

            ListView {
                id:                  pairedList
                width:               parent.width
                height:              contentHeight
                interactive:         false
                clip:                true

                model: SettingsManager.bluetooth.devices

                // Only show paired devices in this section
                delegate: Loader {
                    width: pairedList.width

                    // filter: only paired devices
                    active: model.paired
                    sourceComponent: pairedRow
                    property var rowModel: model
                }

                Component {
                    id: pairedRow

                    SettingsRow {
                        width:       pairedList.width
                        label:       rowModel.name
                        description: rowModel.connected
                                     ? "✓ Connected · " + rowModel.typeHint
                                     : "Paired · "      + rowModel.typeHint
                        showDivider: true

                        // Buttons
                        Row {
                            spacing: Theme.spacingSmall

                            // Connect / Disconnect button
                            Rectangle {
                                width:  rowModel.connected ? 100 : 80
                                height: 30
                                radius: Theme.radiusSmall
                                color:  rowModel.connected ? Theme.danger : Theme.accent

                                Text {
                                    anchors.centerIn: parent
                                    text:           rowModel.connected ? "Disconnect" : "Connect"
                                    color:          Theme.textOnAccent
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (rowModel.connected)
                                            SettingsManager.bluetooth.disconnectDevice(rowModel.address)
                                        else
                                            SettingsManager.bluetooth.connectDevice(rowModel.address)
                                    }
                                }
                            }

                            // Remove (forget) button
                            Rectangle {
                                width:  70
                                height: 30
                                radius: Theme.radiusSmall
                                color:  "transparent"
                                border.color: Theme.textSecondary
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text:           "Remove"
                                    color:          Theme.textSecondary
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: SettingsManager.bluetooth.removeDevice(rowModel.address)
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Nearby / discovered devices ───────────────────────────────────────
        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "NEARBY DEVICES"
            visible: btToggle.checked

            ColumnLayout {
                width:   parent.width
                spacing: Theme.spacingMedium

                // Scan controls row
                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true
                        text:           SettingsManager.bluetooth.statusText
                        color:          Theme.textSecondary
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    // Spinner while discovering
                    BusyIndicator {
                        running:  SettingsManager.bluetooth.discovering
                        visible:  running
                        Layout.preferredWidth:  20
                        Layout.preferredHeight: 20
                    }

                    // Scan / Stop button toggles based on discovery state
                    Rectangle {
                        width:  90
                        height: 32
                        radius: Theme.radiusSmall
                        color:  SettingsManager.bluetooth.discovering
                                ? Theme.danger
                                : Theme.accent

                        Text {
                            anchors.centerIn: parent
                            text:           SettingsManager.bluetooth.discovering
                                            ? "Stop" : "Scan"
                            color:          Theme.textOnAccent
                            font.pixelSize: Theme.fontSizeSmall
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (SettingsManager.bluetooth.discovering)
                                    SettingsManager.bluetooth.stopDiscovery()
                                else
                                    SettingsManager.bluetooth.startDiscovery()
                            }
                        }
                    }
                }

                // Nearby (unpaired) device list
                ListView {
                    id:          nearbyList
                    Layout.fillWidth: true
                    height:      contentHeight
                    interactive: false
                    clip:        true
                    model:       SettingsManager.bluetooth.devices

                    delegate: Loader {
                        width: nearbyList.width

                        // Only show unpaired devices here
                        active: !model.paired
                        sourceComponent: nearbyRow
                        property var rowModel: model
                    }

                    Component {
                        id: nearbyRow

                        SettingsRow {
                            width:       nearbyList.width
                            label:       rowModel.name
                            description: rowModel.typeHint
                            showDivider: true

                            // Pair button
                            Rectangle {
                                width:  70
                                height: 30
                                radius: Theme.radiusSmall
                                color:  Theme.accent

                                Text {
                                    anchors.centerIn: parent
                                    text:           "Pair"
                                    color:          Theme.textOnAccent
                                    font.pixelSize: Theme.fontSizeSmall
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: SettingsManager.bluetooth.pairDevice(rowModel.address)
                                }
                            }
                        }
                    }
                }

                // Empty state
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    visible: nearbyList.count === 0
                             && !SettingsManager.bluetooth.discovering
                    text:           "No nearby devices. Tap Scan to search."
                    color:          Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    topPadding:     Theme.spacingMedium
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
