add_executable(tankgam)

#core source code
target_sources(tankgam PRIVATE
        core/Engine.h core/Engine.cpp
        core/Menu.h core/Menu.cpp
        core/Thing.h core/Thing.cpp
        core/Player.h core/Player.cpp
        core/World.h core/World.cpp
        core/Physics.h core/Physics.cpp
        core/Event.h core/Event.cpp
        core/Renderable.h core/Renderable.cpp
        core/Thinker.h core/Thinker.cpp)

#SDL2 specific source code
target_sources(tankgam PRIVATE
        sdl2/main.cpp
        sdl2/sys/Console.h sdl2/sys/Console.cpp
        sdl2/sys/Renderer.h sdl2/sys/Renderer.cpp
        sdl2/sys/Renderer/Mesh.h sdl2/sys/Renderer/Mesh.cpp
        sdl2/sys/Renderer/Shader.h sdl2/sys/Renderer/Shader.cpp
        sdl2/sys/Renderer/Texture.h sdl2/sys/Renderer/Texture.cpp
        sdl2/sys/Renderer/TextRenderer.h sdl2/sys/Renderer/TextRenderer.cpp
        sdl2/sys/EventHandler.h sdl2/sys/EventHandler.cpp
        sdl2/sys/File.h sdl2/sys/File.cpp
        sdl2/sys/Timer.h sdl2/sys/Timer.cpp
        sdl2/sys/File.h sdl2/sys/File.cpp)

#we want ALL the warnings
if(MSVC)
    target_compile_options(tankgam PRIVATE /W4)
else()
    target_compile_options(tankgam PRIVATE -Wall -Wextra -Wpedantic)
endif()

#C++ settings
target_compile_features(tankgam PUBLIC cxx_std_20)
set_target_properties(tankgam PROPERTIES CXX_EXTENSIONS OFF)

#linking various external files
target_link_libraries(tankgam PRIVATE SDL2::SDL2main OpenGL::GL SDL2::SDL2 glm::glm libzip::zip)

#move up some directories to be up
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/core")
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdl2")

#glad2 files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/include")

#fmt files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/include")

#stb files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/stb/src/stb_image.cpp")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/stb/include")

#tiny obj loader files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/tiny_obj_loader/src/tiny_obj_loader.cpp")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/tiny_obj_loader/include")
