add_library(tankgam-util STATIC)

target_sources(tankgam-util PRIVATE
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/Log.h
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/File.h util/File.cpp)

#we want ALL the warnings
if(${MSVC})
    target_compile_options(tankgam-util PRIVATE /W4)
else()
    target_compile_options(tankgam-util PRIVATE -Wall -Wextra -Wpedantic)
endif()

#C++ settings
target_compile_features(tankgam-util PUBLIC cxx_std_20)
set_target_properties(tankgam-util PROPERTIES CXX_EXTENSIONS OFF)

#linking various external files
target_link_libraries(tankgam-util PUBLIC libzip::zip)

#move up some directories to be up
target_include_directories(tankgam-util PUBLIC "${PROJECT_SOURCE_DIR}/include/tankgam-util")

#fmt files
target_sources(tankgam-util PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-util PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/include")
