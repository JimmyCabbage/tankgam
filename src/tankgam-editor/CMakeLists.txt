qt_add_executable(tankgam-editor
        qt6/main.cpp
        qt6/WorldEditorWindow.h qt6/WorldEditorWindow.cpp
        qt6/ViewportWindow.h qt6/ViewportWindow.cpp
        qt6/RenderWidget.h qt6/RenderWidget.cpp
        qt6/SettingsTab.h qt6/SettingsTab.cpp)

#internal logic stuff
target_sources(tankgam-editor PRIVATE
        logic/Brush.h logic/Brush.cpp
        logic/Viewport.h logic/Viewport.cpp
        logic/ViewportCamera.h logic/ViewportCamera.cpp
        logic/ViewportToolType.h
        logic/ViewportHelpers.h logic/ViewportHelpers.cpp
        logic/Editor.h logic/Editor.cpp
        logic/StdLog.h logic/StdLog.cpp)

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
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/logic")
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/math")
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/qt6")

#glad2 files
target_sources(tankgam-editor PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/src/gl.c")
target_include_directories(tankgam-editor PRIVATE "${CMAKE_SOURCE_DIR}/external/glad2/include")

#fmt files
target_sources(tankgam-editor PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc")
target_include_directories(tankgam-editor PRIVATE "${CMAKE_SOURCE_DIR}/external/fmt/include")
