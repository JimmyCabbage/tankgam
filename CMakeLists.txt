cmake_minimum_required(VERSION 3.16)
project(tankgam VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(glm REQUIRED)
find_package(SDL2 2.0.8 REQUIRED)
find_package(libzip REQUIRED)

add_subdirectory(src)
