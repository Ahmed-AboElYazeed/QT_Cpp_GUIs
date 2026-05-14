pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    // --- Mode ---
    property bool isDark: false

    // --- Surfaces ---
    property color background:     isDark ? "#1e1e2e" : "#f2f2f7"
    property color surface:        isDark ? "#2a2a3e" : "#ffffff"
    property color sidebarBg:      isDark ? "#16161e" : "#e8e8ed"
    property color sidebarHover:   isDark ? "#2a2a3e" : "#d8d8e0"
    property color sidebarActive:  isDark ? "#3d5afe" : "#3d5afe"
    property color divider:        isDark ? "#ffffff18" : "#d8d8e0"

    // --- Text ---
    property color textPrimary:    isDark ? "#e8e8f0" : "#1c1c1e"
    property color textSecondary:  isDark ? "#9999bb" : "#6e6e80"
    property color textOnAccent:   "#ffffff"

    // --- Accent ---
    property color accent:         "#3d5afe"
    property color accentHover:    "#5c77ff"
    property color success:        "#4caf50"
    property color warning:        "#ff9800"
    property color danger:         "#f44336"
    property color surfaceVariant: "#ff9800"

    // --- Radii ---
    property int radiusSmall:  6
    property int radiusMedium: 10
    property int radiusLarge:  16

    // --- Spacing ---
    property int spacingSmall:  8
    property int spacingMedium: 16
    property int spacingLarge:  24

    // --- Typography ---
    property int fontSizeSmall:   12
    property int fontSizeBody:    14
    property int fontSizeTitle:   18
    property int fontSizeHeading: 22
    property int fontWeightNormal: Font.Normal
    property int fontWeightMedium: Font.Medium
    property int fontWeightBold:   Font.Bold

    // --- Sidebar ---
    property int sidebarWidth:    240
    property int sidebarItemH:    44

    // --- Animations ---
    property int animFast:   120
    property int animNormal: 200
}
