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

set(LOCONET2_DIR "" CACHE PATH "Path to LocoNet2 library")

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

        server/loconet/loconetz21adapter.h
        server/loconet/loconetz21adapter.cpp
        server/loconet/loconetz21source.h
        server/loconet/loconetz21source.cpp
        server/loconet/loconettcpbinaryserver.h
        server/loconet/loconettcpbinaryserver.cpp
        server/loconet/loconettcpsource.h
        server/loconet/loconettcpsource.cpp
        server/loconet/loconetbus.h
        server/loconet/loconetslotserver.h
        server/loconet/loconetslotserver.cpp

        widgets/powerstatusled.h
        widgets/powerstatusled.cpp
        widgets/throttlewidget.h
        widgets/throttlewidget.cpp

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

# LocoNet support
if(EXISTS ${LOCONET2_DIR})
    target_include_directories(Z21ServerEmulator PRIVATE ${LOCONET2_DIR})
    target_compile_definitions(Z21ServerEmulator PRIVATE WITH_LOCONET2=1 DEBUG_LN2_OUTPUT=1)
    target_sources(Z21ServerEmulator PRIVATE
        ${LOCONET2_DIR}/LocoNet2.cpp
        ${LOCONET2_DIR}/LocoNetMessageBuffer.cpp)
endif()

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
