cmake_minimum_required(VERSION 3.5)

project(KAmbiLED LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia REQUIRED)

add_executable(KAmbiLED
    src/main.cpp
    src/kambiled.cpp
    src/kambiled.h
    src/HyperionProcessor.cpp
    src/HyperionProcessor.h
    src/wledclient.cpp
    src/wledclient.h
    src/LedColorMapping.cpp
    src/LedColorMapping.h
    src/LinearColorSmoothing.cpp
    src/LinearColorSmoothing.h
    src/BlackBorderDetector.cpp
    src/BlackBorderDetector.h
    src/WaylandGrabber.cpp
    src/WaylandGrabber.h
    src/debugclient.cpp
    src/debugclient.h
    src/wledconfigclient.cpp
    src/wledconfigclient.h
    src/wayland_protocols/qwayland-zkde-screencast-unstable-v1.h
    src/wayland_protocols/zkde-screencast-unstable-v1-client-protocol.h
)

target_link_libraries(KAmbiLED PRIVATE Qt::Core Qt::Gui Qt::Widgets Qt::Multimedia)
