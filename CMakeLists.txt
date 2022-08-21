cmake_minimum_required(VERSION 3.5)

project(http_request VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_PREFIX_PATH $ENV{QTDIR})
set (CMAKE_BUILD_TYPE Release)

# Include project files
set(PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/includes")

file(GLOB SOURCES
    "${PROJECT_SOURCES}/*.cpp"
    "${INCLUDE_DIR}/*.h"
)

include_directories(${PROJECT_SOURCES})
include_directories(${INCLUDE_DIR})
include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network REQUIRED)

# Add shared library
add_library(http_request ${SOURCES})

target_link_libraries(http_request PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(http_request PRIVATE Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(http_request PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER includes/HttpRequest.h
)


configure_file(http_request.pc.in http_request.pc @ONLY)
target_include_directories(http_request PUBLIC .)
install(TARGETS http_request
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${CMAKE_BINARY_DIR}/http_request.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)