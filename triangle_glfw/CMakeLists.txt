cmake_minimum_required(VERSION 3.16)
project(vulkantriangle_glfw LANGUAGES CXX)

find_package(Vulkan REQUIRED)
#find_package(glfw3 3.3 REQUIRED)
# find_package(SDL3   REQUIRED)
find_program(GLSLC_PROGRAM glslc REQUIRED)

# Where our GLSL lives, and where we output SPV
set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR})

# Compile vertex
add_custom_command(
  OUTPUT ${SHADER_OUT}/vert.spv
  COMMAND ${GLSLC_PROGRAM}
          -fshader-stage=vertex
          ${SHADER_SRC}/vert.vert
          -o ${SHADER_OUT}/vert.spv
  DEPENDS ${SHADER_SRC}/vert.vert
)

# Compile fragment
add_custom_command(
  OUTPUT ${SHADER_OUT}/frag.spv
  COMMAND ${GLSLC_PROGRAM}
          -fshader-stage=fragment
          ${SHADER_SRC}/frag.frag
          -o ${SHADER_OUT}/frag.spv
  DEPENDS ${SHADER_SRC}/frag.frag
)

# A phony target to build both
add_custom_target(triangleglfwShaders
  DEPENDS
    ${SHADER_OUT}/vert.spv
    ${SHADER_OUT}/frag.spv
)

# Our executable
add_executable(vulkantriangle_glfw
  triangle.cpp
)

# Make sure shaders are built before the exe
add_dependencies(vulkantriangle_glfw triangleglfwShaders)

# Link and include
target_include_directories(vulkantriangle_glfw PRIVATE
  ${Vulkan_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/include
)
target_link_libraries(vulkantriangle_glfw PRIVATE
  glfw
  Vulkan::Vulkan
)

# After building the exe, copy SPV files into the same folder
add_custom_command(TARGET vulkantriangle_glfw POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SHADER_OUT}/vert.spv
    ${SHADER_OUT}/frag.spv
    $<TARGET_FILE_DIR:vulkantriangle_glfw>
)
