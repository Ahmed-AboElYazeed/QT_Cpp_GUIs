cmake_minimum_required(VERSION 3.16)

project(settings VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)       # auto-run moc on Q_OBJECT classes
set(CMAKE_AUTORCC ON)       # auto-compile .qrc resource files

# ── Qt modules ───────────────────────────────────────────────────────────────
# Add Qt6::Bluetooth, Qt6::Multimedia etc. as you implement each manager.
find_package(ALSA REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick          # QQuickView / QML engine
    DBus
    # Bluetooth    # ← add for BluetoothManager
    # Multimedia   # ← add for VolumeManager
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appsettings
    main.cpp
)

qt_add_qml_module(appsettings
    URI settings
    QML_FILES
        Main.qml
        QML_FILES qml/theme/Theme.qml
        QML_FILES qml/sidebar/Sidebar.qml
        QML_FILES qml/sidebar/SidebarItem.qml
        QML_FILES qml/components/PageHeader.qml
        QML_FILES qml/components/SettingsGroup.qml
        QML_FILES qml/components/SettingsRow.qml
        QML_FILES qml/components/StyledToggle.qml
        QML_FILES qml/components/StyledSlider.qml
        QML_FILES qml/pages/WifiPage.qml
        QML_FILES qml/pages/BluetoothPage.qml
        QML_FILES qml/pages/VolumePage.qml
        QML_FILES qml/pages/ThemePage.qml
        RESOURCES qml/theme/qmldir
        SOURCES
        SOURCES
        SOURCES settingsbackend/settingsmanager.h settingsbackend/settingsmanager.cpp
        SOURCES settingsbackend/wifimanager.h settingsbackend/wifimanager.cpp
        SOURCES
        SOURCES
        SOURCES settingsbackend/soundmanager.h settingsbackend/soundmanager.cpp
        SOURCES settingsbackend/bluetoothmanager.h settingsbackend/bluetoothmanager.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appsettings PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appsettings
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsettings
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::DBus
        ALSA::ALSA
        # Qt6::Bluetooth
        # Qt6::Multimedia
    )

include(GNUInstallDirs)
install(TARGETS appsettings
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
