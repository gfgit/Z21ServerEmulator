cmake_minimum_required(VERSION 3.5)

project(Z21ServerEmulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        server/z21server.cpp
        server/z21server.h
        server/z21server_constants.h

        server/retroaction/rbusretroaction.h
        server/retroaction/rbusretroaction.cpp
        server/retroaction/rbusinputmodel.h
        server/retroaction/rbusinputmodel.cpp

        server/accessories/accessorymanager.h
        server/accessories/accessorymanager.cpp
        server/accessories/accessorymodel.h
        server/accessories/accessorymodel.cpp

        server/loco/locomanager.h
        server/loco/locomanager.cpp
        server/loco/locodrivemodel.h
        server/loco/locodrivemodel.cpp

        widgets/powerstatusled.h
        widgets/powerstatusled.cpp

        arduino_compat/WProgram.h
        arduino_compat/EEPROM.h
        arduino_compat/arduinocompat.h
        arduino_compat/arduinocompat.cpp

        z21library/z21header.h
        z21library/z21.h
        z21library/z21.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Z21ServerEmulator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Z21ServerEmulator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Z21ServerEmulator SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Z21ServerEmulator
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(Z21ServerEmulator PRIVATE z21library arduino_compat)

target_link_libraries(Z21ServerEmulator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(Z21ServerEmulator PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Z21ServerEmulator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Z21ServerEmulator)
endif()
