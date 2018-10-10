cmake_minimum_required(VERSION 3.4)
project(ModpackCreator)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Tidy REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
