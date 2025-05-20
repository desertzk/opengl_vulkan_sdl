cmake_minimum_required(VERSION 3.10)
project(opengl_camera_glfw)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRC
    camera_mouse_zoom.cpp
    ../vendored/glad/src/glad.c
)

# Shader files (will be copied to output directory)
set(SHADERS
    7.3.camera.vs
    7.3.camera.fs
)

# Find dependencies
find_package(OpenGL REQUIRED)


# Include directories (assuming vendored libraries are in ../vendored)
include_directories(
    ${PROJECT_SOURCE_DIR}/../vendored/glad/include
    ${PROJECT_SOURCE_DIR}/../vendored/glfw/include
    ${PROJECT_SOURCE_DIR}/../vendored/glm
    ${PROJECT_SOURCE_DIR}/../vendored/stb
)



# Create executable
add_executable(${PROJECT_NAME} ${SRC})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        OpenGL::GL
)

# Copy shaders to output directory
foreach(_shader IN LISTS SHADERS)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${_shader}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${_shader}"
    )
endforeach()

# --- copy entire textures folder ---
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/textures"
)
