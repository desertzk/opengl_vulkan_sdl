cmake_minimum_required(VERSION 3.15)
project(VideoPlayer LANGUAGES CXX)

find_package(Vulkan REQUIRED)
# find_package(SDL3   REQUIRED)

# --- build your exe ---
add_executable(VideoPlayer
  v4l2_vulkan_video.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
  ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
)

target_link_libraries(VideoPlayer PRIVATE
  SDL3::SDL3
  Vulkan::Vulkan
)

# --- compile shaders ---
set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${SHADER_OUT}/vert.spv
  COMMAND ${Vulkan_GLSLC_EXECUTABLE}
          ${SHADER_SRC}/vert.vert
          -o ${SHADER_OUT}/vert.spv
  DEPENDS ${SHADER_SRC}/vert.vert
)

add_custom_command(
  OUTPUT ${SHADER_OUT}/frag.spv
  COMMAND ${Vulkan_GLSLC_EXECUTABLE}
          ${SHADER_SRC}/frag.frag
          -o ${SHADER_OUT}/frag.spv
  DEPENDS ${SHADER_SRC}/frag.frag
)

add_custom_target(Shaders
  DEPENDS
    ${SHADER_OUT}/vert.spv
    ${SHADER_OUT}/frag.spv
)

add_dependencies(VideoPlayer Shaders)
