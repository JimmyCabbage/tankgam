add_executable(tankgam)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Version.h.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Version.h"
)

#copy assets files to build directory
configure_file("${PROJECT_SOURCE_DIR}/dev.assets" "${CMAKE_CURRENT_BINARY_DIR}/dev.assets" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tank.assets" "${CMAKE_CURRENT_BINARY_DIR}/tank.assets" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/base_textures.assets" "${CMAKE_CURRENT_BINARY_DIR}/base_textures.assets" COPYONLY)

#core source code
target_sources(tankgam PRIVATE
        src/core/Client/Menu.h src/core/Client/Menu.cpp
        src/core/Event.h src/core/Event.cpp
        src/core/Client.h src/core/Client.cpp
        src/core/Server.h src/core/Server.cpp
        src/core/NetChan.h src/core/NetChan.cpp
        src/core/NetBuf.h src/core/NetBuf.cpp
        src/core/Entity.h src/core/Entity.cpp
        src/core/EntityManager.h src/core/EntityManager.cpp
        src/core/Net.h src/core/Net.cpp
        src/core/Client/ClientMenuState.h src/core/Client/ClientMenuState.cpp
        src/core/Client/ClientConnectingState.h src/core/Client/ClientConnectingState.cpp
        src/core/Client/ClientConnectedState.h src/core/Client/ClientConnectedState.cpp
        src/core/Client/IClientState.h
        src/core/Version.h)

#linux specific source code
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources(tankgam PRIVATE
            src/linux/sys/NetLoopback.h src/linux/sys/NetLoopback.cpp)
endif()

#windows specific source code
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_sources(tankgam PRIVATE
            src/win32/sys/NetLoopback.h src/win32/sys/NetLoopback.cpp)
endif()


#SDL2 specific source code
target_sources(tankgam PRIVATE
        src/sdl2/main.cpp
        src/sdl2/sys/Console.h src/sdl2/sys/Console.cpp
        src/sdl2/sys/Renderer.h src/sdl2/sys/Renderer.cpp
        src/sdl2/sys/Renderer/TextRenderer.h src/sdl2/sys/Renderer/TextRenderer.cpp
        src/sdl2/sys/EventHandler.h src/sdl2/sys/EventHandler.cpp
        src/sdl2/sys/Timer.h src/sdl2/sys/Timer.cpp)

#we want ALL the warnings
if(${MSVC})
    target_compile_options(tankgam PRIVATE /W4)
else()
    target_compile_options(tankgam PRIVATE -Wall -Wextra -Wpedantic)
    #debugging
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(tankgam PRIVATE -g)
    endif()
endif()

#C++ settings
target_compile_features(tankgam PUBLIC cxx_std_20)
set_target_properties(tankgam PROPERTIES CXX_EXTENSIONS OFF)

#linking various internal files
target_link_libraries(tankgam PRIVATE tankgam-gl tankgam-util)

#linking various external files
target_link_libraries(tankgam PRIVATE SDL2::SDL2main SDL2::SDL2 glm::glm)

#link JoltPhysics manually
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(tankgam PRIVATE Jolt)
endif()

#move up some directories to be up
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/core")
target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/sdl2")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/linux")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(tankgam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/win32")
endif()

#glad2 files
target_sources(tankgam PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/include")

#fmt files
target_sources(tankgam PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/include")

include(GNUInstallDirs)

#install commands
install(TARGETS tankgam)
install(FILES
    "${PROJECT_SOURCE_DIR}/dev.assets"
    "${PROJECT_SOURCE_DIR}/base_textures.assets"
    "${PROJECT_SOURCE_DIR}/tank.assets"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/tankgam"
)
