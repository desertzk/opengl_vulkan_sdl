cmake_minimum_required(VERSION 3.10)
project(opengl_triangle)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRC
    triangle.cpp
    ../vendored/glad/src/glad.c
)



# Find dependencies
find_package(OpenGL REQUIRED)


# Include directories (assuming vendored libraries are in ../vendored)
include_directories(
    ${PROJECT_SOURCE_DIR}/../vendored/glad/include
    ${PROJECT_SOURCE_DIR}/../vendored/glfw/include
)



# Create executable
add_executable(${PROJECT_NAME} ${SRC})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        OpenGL::GL
)


