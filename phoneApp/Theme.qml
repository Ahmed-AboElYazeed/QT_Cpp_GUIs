pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    property bool isDark: true

    // Surfaces
    property color bgSurface:   isDark ? "#1e1e2e" : "#f2f2f7"
    property color bgFooter:    isDark ? "#16161e" : "#e8e8ed"
    property color bgCard:      isDark ? "#2a2a3e" : "#ffffff"
    property color bgHover:     isDark ? "#2a2a3e" : "#d8d8e0"
    property color bgDeep:      isDark ? "#11111a" : "#d1d1d6"

    // Borders
    property color b1:          isDark ? "#ffffff18" : "#d8d8e0"
    property color b2:          isDark ? "#ffffff30" : "#c7c7cc"

    // Text
    property color t0:          isDark ? "#e8e8f0" : "#1c1c1e"
    property color t1:          isDark ? "#9999bb" : "#6e6e80"
    property color t2:          isDark ? "#666688" : "#8e8e93"

    // Phone accents
    property color phoneGlow:   isDark ? "#3d5afe20" : "#3d5afe15"
    property color phoneBord:   "#3d5afe"
    property color phoneAc:     "#3d5afe"

    // Radii
    property int r1:    6
    property int r2:    10
    property int rFull: 999

    // Timing
    property int fast: 120
}