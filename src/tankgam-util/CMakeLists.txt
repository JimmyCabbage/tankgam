add_library(tankgam-util STATIC)

target_sources(tankgam-util PRIVATE
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/Log.h util/Log.cpp
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/FileManager.h util/FileManager.cpp
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/Plane.h util/Plane.cpp
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/Brush.h util/Brush.cpp
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/BspBuilder.h util/BspBuilder.cpp
        ${PROJECT_SOURCE_DIR}/include/tankgam-util/util/Bsp.h util/Bsp.cpp)

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
target_link_libraries(tankgam-util PUBLIC libzip::zip glm::glm)

#move up some directories to be up
target_include_directories(tankgam-util PUBLIC "${PROJECT_SOURCE_DIR}/include/tankgam-util")

#fmt files
target_sources(tankgam-util PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-util PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/include")
