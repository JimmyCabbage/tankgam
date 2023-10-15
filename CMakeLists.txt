cmake_minimum_required(VERSION 3.16)
project(tankgam VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Qt6 stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

find_package(glm REQUIRED)
find_package(SDL2 2.0.8 REQUIRED)
find_package(libzip REQUIRED)

add_subdirectory(src)
