cmake_minimum_required(VERSION 3.10)
project(opengl_basaic_light)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRC
    basic_lighting_specular.cpp
    ../vendored/glad/src/glad.c
)

# Shader files (will be copied to output directory)
set(SHADERS
    2.2.basic_lighting.vs
    2.2.basic_lighting.fs
    2.2.light_cube.fs
    2.2.light_cube.vs
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


