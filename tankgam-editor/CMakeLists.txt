qt_add_executable(tankgam-editor
        src/qt6/main.cpp
        src/qt6/WorldEditorWindow.h src/qt6/WorldEditorWindow.cpp
        src/qt6/ViewportWindow.h src/qt6/ViewportWindow.cpp
        src/qt6/SettingsTab.h src/qt6/SettingsTab.cpp)

#copy assets files to build directory
configure_file("${PROJECT_SOURCE_DIR}/dev.assets" "${CMAKE_CURRENT_BINARY_DIR}/dev.assets" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tank.assets" "${CMAKE_CURRENT_BINARY_DIR}/tank.assets" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/base_textures.assets" "${CMAKE_CURRENT_BINARY_DIR}/base_textures.assets" COPYONLY)

#internal logic stuff
target_sources(tankgam-editor PRIVATE
        src/logic/Viewport.h src/logic/Viewport.cpp
        src/logic/ViewportCamera.h src/logic/ViewportCamera.cpp
        src/logic/ViewportToolType.h
        src/logic/ViewportHelpers.h src/logic/ViewportHelpers.cpp
        src/logic/Editor.h src/logic/Editor.cpp
        src/logic/StdLog.h src/logic/StdLog.cpp)

#we want ALL the warnings
if(MSVC)
    target_compile_options(tankgam-editor PRIVATE /W4)
else()
    target_compile_options(tankgam-editor PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()

#C++ settings
target_compile_features(tankgam-editor PUBLIC cxx_std_20)
set_target_properties(tankgam-editor PROPERTIES CXX_EXTENSIONS OFF)

#GUI settings for Qt6
set_target_properties(tankgam-editor PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

#linking various internal files
target_link_libraries(tankgam-editor PRIVATE tankgam-gl tankgam-util)

#linking various external libraries
target_link_libraries(tankgam-editor PRIVATE Qt::Core Qt::Gui Qt::Widgets glm::glm)

#move up some directories to be up
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/logic/")
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/qt6/")

#glad2 files
target_sources(tankgam-editor PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam-editor PRIVATE "${PROJECT_SOURCE_DIR}/external/glad2/include")

#fmt files
target_sources(tankgam-editor PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-editor PRIVATE "${PROJECT_SOURCE_DIR}/external/fmt/include")
