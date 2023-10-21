cmake_minimum_required(VERSION 3.16)
project(tankgam VERSION 0.3.0 LANGUAGES C CXX)

option(BUILD_EDITOR "Build the tankgam editor" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(BUILD_EDITOR)
    #Qt6 stuff
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
endif(BUILD_EDITOR)

find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(libzip REQUIRED)

add_subdirectory(src)
