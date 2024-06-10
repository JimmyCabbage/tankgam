add_library(tankgam-util STATIC)

target_sources(tankgam-util PRIVATE
        include/util/Log.h src/util/Log.cpp
        include/util/FileManager.h src/util/FileManager.cpp
        include/util/Plane.h src/util/Plane.cpp
        include/util/Brush.h src/util/Brush.cpp
        include/util/BspBuilder.h src/util/BspBuilder.cpp
        include/util/Bsp.h src/util/Bsp.cpp)

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
target_link_libraries(tankgam-util PUBLIC glm::glm)
target_link_libraries(tankgam-util PRIVATE libzip::zip)

#move up some directories to be up
target_include_directories(tankgam-util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(tankgam-util PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")

#fmt files
target_sources(tankgam-util PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-util PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/include")
