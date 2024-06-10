add_library(tankgam-gl STATIC)

target_sources(tankgam-gl PRIVATE
        include/gl/Vertex.h
        include/gl/Mesh.h src/gl/Mesh.cpp
        include/gl/Shader.h src/gl/Shader.cpp
        include/gl/Texture.h src/gl/Texture.cpp)

#we want ALL the warnings
if(${MSVC})
    target_compile_options(tankgam-gl PRIVATE /W4)
else()
    target_compile_options(tankgam-gl PRIVATE -Wall -Wextra -Wpedantic)
endif()

#C++ settings
target_compile_features(tankgam-gl PUBLIC cxx_std_20)
set_target_properties(tankgam-gl PROPERTIES CXX_EXTENSIONS OFF)

#linking various external files
target_link_libraries(tankgam-gl PUBLIC glm::glm)

#move up some directories to be up
target_include_directories(tankgam-gl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(tankgam-gl PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")

#tiny obj loader files
target_sources(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/tiny_obj_loader/src/tiny_obj_loader.cpp")
target_include_directories(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/tiny_obj_loader/include")

#glad2 files
target_sources(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/include")

#stb files
target_sources(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/stb/src/stb_image.cpp")
target_include_directories(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/stb/include")

#fmt files
target_sources(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-gl PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/include")
