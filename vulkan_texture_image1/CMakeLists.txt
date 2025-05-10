cmake_minimum_required(VERSION 3.14)
project(texture_image_image1 LANGUAGES CXX)

# ----------------------------------------------------------------------------
# 1) C++ Standard
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# 3) Find Vulkan
# ----------------------------------------------------------------------------
find_package(Vulkan REQUIRED)

# ----------------------------------------------------------------------------
# 4) Find GLFW (instead of SDL3)
# ----------------------------------------------------------------------------
# find_package(glfw3 3.3 REQUIRED)  # ← replaces SDL3 find_package :contentReference[oaicite:0]{index=0}

# ----------------------------------------------------------------------------
# 5) Shader compilation
# ----------------------------------------------------------------------------
find_program(GLSLC_PROGRAM glslc REQUIRED)

set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${SHADER_OUT}/vert.spv
  COMMAND ${GLSLC_PROGRAM}
          ${SHADER_SRC}/vert.vert
          -o ${SHADER_OUT}/vert.spv
  DEPENDS ${SHADER_SRC}/vert.vert
)

add_custom_command(
  OUTPUT ${SHADER_OUT}/frag.spv
  COMMAND ${GLSLC_PROGRAM}
          ${SHADER_SRC}/frag.frag
          -o ${SHADER_OUT}/frag.spv
  DEPENDS ${SHADER_SRC}/frag.frag
)

add_custom_target(texture_image_image1_shaders
  DEPENDS
    ${SHADER_OUT}/vert.spv
    ${SHADER_OUT}/frag.spv
)

# ----------------------------------------------------------------------------
# 6) Sources & Executable
# ----------------------------------------------------------------------------
set(SRC
    main.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

# ----------------------------------------------------------------------------
# 7) Include directories
# ----------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../vendored/stb
    ${PROJECT_SOURCE_DIR}/../vendored/glm/
    # GLFW’s headers are provided automatically by find_package :contentReference[oaicite:1]{index=1}
)


# Make sure shaders build first
add_dependencies(${PROJECT_NAME} texture_image_image1_shaders)

# ----------------------------------------------------------------------------
# 8) Link libraries
# ----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw          # ← link against GLFW instead of SDL3 :contentReference[oaicite:2]{index=2}
)

file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/textures
  DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR})
