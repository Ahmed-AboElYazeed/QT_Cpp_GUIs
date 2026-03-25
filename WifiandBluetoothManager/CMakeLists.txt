cmake_minimum_required(VERSION 3.16)
project(WifiBluetoothManager VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick DBus Bluetooth Network)

qt_add_executable(appWifiBluetoothManager
    main.cpp
    controller/AppController.cpp
    wifi/WifiManager.cpp
    wifi/WifiNetworkModel.cpp
    bluetooth/BluetoothManager.cpp
    bluetooth/BluetoothDeviceModel.cpp
    settings/SettingsManager.cpp
)

qt_add_qml_module(appWifiBluetoothManager
    URI WifiandBluetoothManager
    VERSION 1.0
    QML_FILES
        Main.qml
        WifiPage.qml
        BluetoothPage.qml
        NetworkCard.qml
        DeviceCard.qml
        ToggleSwitch.qml
        SOURCES wifi/WifiNetworkModel.h
        SOURCES wifi/WifiManager.h
        SOURCES settings/SettingsManager.h
        SOURCES controller/AppController.h
        SOURCES bluetooth/BluetoothManager.h bluetooth/BluetoothDeviceModel.h
)

target_link_libraries(appWifiBluetoothManager
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::DBus
        Qt6::Bluetooth
        Qt6::Network
)
