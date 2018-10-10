cmake_minimum_required(VERSION 3.4)
project(ModpackCreator)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
