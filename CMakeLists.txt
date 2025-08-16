cmake_minimum_required(VERSION 3.10.0)

project(Hyperion_Grabber_Wayland_QT VERSION 0.2 LANGUAGES C CXX)

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

add_executable(Hyperion_Grabber_Wayland_QT
    hyperiongrabber.cpp
    wledclient.cpp
    debugclient.cpp # Added debugclient.cpp
    main.cpp
    WaylandGrabber.cpp
    wledconfigclient.cpp
    LedColorMapping.cpp
    BlackBorderDetector.cpp
    HyperionProcessor.cpp
    LinearColorSmoothing.cpp
)

target_link_libraries(Hyperion_Grabber_Wayland_QT
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

add_executable(hyperion-mock
    hyperion-mock.cpp
)

target_link_libraries(hyperion-mock
    Qt6::Core
    Qt6::Network
    Qt6::Gui
)



add_executable(screen_capture_test
    screen_capture_test.cpp
)

target_link_libraries(screen_capture_test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::MultimediaWidgets
)


find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

set(WAYLAND_PROTOCOLS_DIR "${CMAKE_CURRENT_BINARY_DIR}/wayland_protocols")
file(MAKE_DIRECTORY ${WAYLAND_PROTOCOLS_DIR})

add_custom_command(
    OUTPUT ${WAYLAND_PROTOCOLS_DIR}/qwayland-zkde-screencast-unstable-v1.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${CMAKE_CURRENT_SOURCE_DIR}/cloned_repos/plasma-wayland-protocols/src/protocols/zkde-screencast-unstable-v1.xml" ${WAYLAND_PROTOCOLS_DIR}/qwayland-zkde-screencast-unstable-v1.h
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cloned_repos/plasma-wayland-protocols/src/protocols/zkde-screencast-unstable-v1.xml"
    COMMENT "Generating Wayland client header for zkde-screencast-unstable-v1"
)

add_custom_command(
    OUTPUT ${WAYLAND_PROTOCOLS_DIR}/zkde-screencast-unstable-v1-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code "${CMAKE_CURRENT_SOURCE_DIR}/cloned_repos/plasma-wayland-protocols/src/protocols/zkde-screencast-unstable-v1.xml" ${WAYLAND_PROTOCOLS_DIR}/zkde-screencast-unstable-v1-client-protocol.h
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cloned_repos/plasma-wayland-protocols/src/protocols/zkde-screencast-unstable-v1.xml"
    COMMENT "Generating Wayland client protocol code for zkde-screencast-unstable-v1"
)

add_custom_target(generate_wayland_protocols ALL
    DEPENDS ${WAYLAND_PROTOCOLS_DIR}/qwayland-zkde-screencast-unstable-v1.h
            ${WAYLAND_PROTOCOLS_DIR}/zkde-screencast-unstable-v1-client-protocol.h
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
