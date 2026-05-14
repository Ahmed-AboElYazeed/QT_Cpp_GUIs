import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../theme"
import "../components"

Item {
    ColumnLayout {
        anchors {
            fill: parent
            margins: Theme.spacingLarge
        }
        spacing: Theme.spacingMedium

        PageHeader {
            title: "Appearance"
            subtitle: "Customize the look of your interface"
            Layout.fillWidth: true
        }

        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "COLOR SCHEME"

            SettingsRow {
                label: "Dark Mode"
                description: "Switch between light and dark interface"
                showDivider: false

                StyledToggle {
                    checked: Theme.isDark
                    onToggled: (val) => {
                        Theme.isDark = val
                        console.log("Dark mode:", val)
                    }
                }
            }
        }

        // Live preview card
        SettingsGroup {
            Layout.fillWidth: true
            groupTitle: "PREVIEW"

            SettingsRow {
                label: "Sample text in primary color"
                description: "This is how secondary text looks"
                showDivider: true

                StyledToggle { checked: true }
            }

            SettingsRow {
                label: "Sample slider"
                showDivider: false

                StyledSlider {
                    from: 0; to: 100; value: 60
                    width: 180
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
