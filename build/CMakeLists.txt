cmake_minimum_required(VERSION 3.10.0)

project(KAmbiLED VERSION 0.2 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable DebugClient
option(USE_DEBUG_CLIENT "Use DebugClient for terminal output instead of WLEDClient" OFF)
if(USE_DEBUG_CLIENT)
    add_compile_definitions(DEBUG_MODE)
endif()

find_package(Qt6 COMPONENTS Core Gui WaylandClient Multimedia Widgets MultimediaWidgets Network OpenGL REQUIRED)

add_executable(KAmbiLED
    ../src/kambiled.cpp
    ../src/wledclient.cpp
    ../src/debugclient.cpp # Added debugclient.cpp
    ../src/main.cpp
    ../src/WaylandGrabber.cpp
    ../src/wledconfigclient.cpp
    ../src/LedColorMapping.cpp
    ../src/BlackBorderDetector.cpp
    ../src/HyperionProcessor.cpp
    ../src/LinearColorSmoothing.cpp
)

target_link_libraries(KAmbiLED
    Qt6::Core
    Qt6::Gui
    Qt6::WaylandClient
    Qt6::Multimedia
    Qt6::Widgets
    Qt6::MultimediaWidgets
    Qt6::Network
    Qt6::OpenGL
    ${PIPEWIRE_LIBRARIES}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)



