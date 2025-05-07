cmake_minimum_required(VERSION 3.14)
project(VulkanTextureDemo LANGUAGES CXX)

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
# 5) Shader compilation
# ----------------------------------------------------------------------------
# find_program(GLSLANG_VALIDATOR glslangValidator)
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

add_custom_target(texture_image_shaders
  DEPENDS
    ${SHADER_OUT}/vert.spv
    ${SHADER_OUT}/frag.spv
)

# ----------------------------------------------------------------------------
# 6) Sources & Executable
# ----------------------------------------------------------------------------
set(SRC
    main.cpp
    # vulkan_helpers.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

# ----------------------------------------------------------------------------
# 7) Include directories
# ----------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/../vendored/stb
    ${PROJECT_SOURCE_DIR}/../vendored/SDL/include
    ${PROJECT_SOURCE_DIR}/../vendored/glm/
)

# Make sure shaders build first
add_dependencies(${PROJECT_NAME} texture_image_shaders)

# ----------------------------------------------------------------------------
# 8) Link libraries
# ----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
)