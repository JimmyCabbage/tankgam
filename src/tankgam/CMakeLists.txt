add_executable(tankgam)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/core/Version.h.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/Version.h"
)

#core source code
target_sources(tankgam PRIVATE
        core/Menu.h core/Menu.cpp
        core/Event.h core/Event.cpp
        core/Client.h core/Client.cpp
        core/Server.h core/Server.cpp
        core/NetChan.h core/NetChan.cpp
        core/NetBuf.h core/NetBuf.cpp
        core/Entity.h core/Entity.cpp
        core/EntityManager.h core/EntityManager.cpp
        core/Net.h core/Net.cpp
        core/Version.h)

#linux specific source code
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources(tankgam PRIVATE
            linux/sys/NetLoopback.h linux/sys/NetLoopback.cpp)
endif()

#windows specific source code
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_sources(tankgam PRIVATE
            win32/sys/NetLoopback.h win32/sys/NetLoopback.cpp)
endif()


#SDL2 specific source code
target_sources(tankgam PRIVATE
        sdl2/main.cpp
        sdl2/sys/Console.h sdl2/sys/Console.cpp
        sdl2/sys/Renderer.h sdl2/sys/Renderer.cpp
        sdl2/sys/Renderer/TextRenderer.h sdl2/sys/Renderer/TextRenderer.cpp
        sdl2/sys/EventHandler.h sdl2/sys/EventHandler.cpp
        sdl2/sys/Timer.h sdl2/sys/Timer.cpp)

#we want ALL the warnings
if(${MSVC})
    target_compile_options(tankgam PRIVATE /W4)
else()
    target_compile_options(tankgam PRIVATE -Wall -Wextra -Wpedantic)
endif()

#C++ settings
target_compile_features(tankgam PUBLIC cxx_std_20)
set_target_properties(tankgam PROPERTIES CXX_EXTENSIONS OFF)

#linking various internal files
target_link_libraries(tankgam PRIVATE tankgam-gl tankgam-util)

#linking various external files
target_link_libraries(tankgam PRIVATE SDL2::SDL2main SDL2::SDL2 glm::glm)

#move up some directories to be up
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/core")
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdl2")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/linux")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/win32")
endif()

#glad2 files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/include")

#fmt files
target_sources(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/include")
