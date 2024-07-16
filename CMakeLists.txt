cmake_minimum_required(VERSION 3.22.0)
project(ToyPlayer VERSION 0.1.0)


cmake_policy(SET CMP0074 NEW)

if (MSVC)
    add_compile_options("/utf-8")
    add_compile_options("/arch:AVX2")
    add_compile_options("/GL")
    add_compile_options("/MP")
    add_link_options("/LTCG")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt库
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

# SQLite3
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIR})
link_directories(${SQLite3_LIBRARY})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

add_subdirectory(src)
include_directories(inc)

qt_add_executable(ToyPlayer
    MANUAL_FINALIZATION
    src/main.cpp
    inc/ui/mainwindow.hpp
    inc/base/VideoWidget.hpp
    inc/ui/VideoDock.hpp
    inc/ui/MediaListDialog.hpp
    inc/ui/ListContextMenu.hpp
    inc/ui/ListSelectDialog.hpp
    ${_UI_SOURCES}
    ${_BASE_SOURCES}
)

target_link_libraries(ToyPlayer PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets SQLite3)

set_target_properties(ToyPlayer PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(ToyPlayer)