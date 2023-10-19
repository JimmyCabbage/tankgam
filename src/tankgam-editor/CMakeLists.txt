qt_add_executable(tankgam-editor qt6/main.cpp
        qt6/WorldEditor.h qt6/WorldEditor.cpp)

#we want ALL the warnings
if(MSVC)
    target_compile_options(tankgam-editor PRIVATE /W4)
else()
    target_compile_options(tankgam-editor PRIVATE -Wall -Wextra -Wpedantic)
endif()

#C++ settings
target_compile_features(tankgam-editor PUBLIC cxx_std_20)
set_target_properties(tankgam-editor PROPERTIES CXX_EXTENSIONS OFF)

#GUI settings for Qt6
set_target_properties(tankgam-editor PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

#linking various external libraries
target_link_libraries(tankgam-editor PRIVATE Qt::Core Qt::Gui Qt::Widgets)

#move up some directories to be up
target_include_directories(tankgam-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/qt6")
