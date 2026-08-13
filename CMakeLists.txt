cmake_minimum_required(VERSION 3.16)
project(Taskmanager-Harbor VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    set(CMAKE_CXX_STANDARD 17)
    add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

if(MSVC)
    add_compile_options(/permissive)
endif()

set(SOURCES
    src/main.cpp
    src/backend/MetricsCollector.cpp
    src/backend/ProcessManager.cpp
    src/ui/FlowLayout.cpp
    src/ui/GraphWidget.cpp
    src/ui/ApplicationsView.cpp
    src/ui/ProcessView.cpp
    src/ui/CpuView.cpp
    src/ui/GpuView.cpp
    src/ui/MemoryView.cpp
    src/ui/NetworkView.cpp
    src/ui/DiskView.cpp
    src/ui/SystemInfoView.cpp
    src/ui/MainWindow.cpp
)

set(HEADERS
    src/backend/SystemMetrics.hpp
    src/backend/MetricsCollector.hpp
    src/backend/ProcessManager.hpp
    src/ui/FlowLayout.hpp
    src/ui/GraphWidget.hpp
    src/ui/ApplicationsView.hpp
    src/ui/ProcessView.hpp
    src/ui/CpuView.hpp
    src/ui/GpuView.hpp
    src/ui/MemoryView.hpp
    src/ui/NetworkView.hpp
    src/ui/DiskView.hpp
    src/ui/SystemInfoView.hpp
    src/ui/MainWindow.hpp
)

set(RESOURCES
    resources.qrc
)

add_executable(Taskmanager-Harbor ${SOURCES} ${HEADERS} ${RESOURCES})

if(WIN32)
    target_link_libraries(Taskmanager-Harbor PRIVATE Qt6::Widgets Qt6::Svg psapi iphlpapi)
else()
    target_link_libraries(Taskmanager-Harbor PRIVATE Qt6::Widgets Qt6::Svg)
endif()

target_include_directories(Taskmanager-Harbor PRIVATE src)
