add_subdirectory(tankgam-gl)
add_subdirectory(tankgam-util)

add_subdirectory(tankgam)

if(TANKGAM_BUILD_EDITOR)
    add_subdirectory(tankgam-editor)
endif(TANKGAM_BUILD_EDITOR)
