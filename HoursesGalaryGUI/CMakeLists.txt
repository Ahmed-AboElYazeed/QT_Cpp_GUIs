cmake_minimum_required(VERSION 3.16)
project(HoursesGalaryGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appHoursesGalaryGUI
    main.cpp
    systeminfo.h
)

qt_add_qml_module(appHoursesGalaryGUI
    URI HoursesGalaryGUI
    VERSION 1.0
    QML_FILES
        Main.qml
        SplashScreen.qml
        HomePage.qml
        GalleryPage.qml
        AboutPopup.qml
        ProductPopup.qml
    RESOURCES
        HG_resources.qrc
)

set_target_properties(appHoursesGalaryGUI PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appHoursesGalaryGUI
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appHoursesGalaryGUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
