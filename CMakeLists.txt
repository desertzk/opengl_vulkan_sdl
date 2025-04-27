cmake_minimum_required(VERSION 3.16)
project(vulkantriangle)

# Add these lines to find Vulkan
find_package(Vulkan REQUIRED)
# find libshaderc (installed with Vulkan SDK, or build from https://github.com/google/shaderc)
#find_package(shaderc_combined REQUIRED)

find_program(GLSLC_PROGRAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/vert.vert -o ${CMAKE_SOURCE_DIR}/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/frag.frag -o ${CMAKE_SOURCE_DIR}/frag.spv)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(vulkantriangle  triangle.cpp)

# Add Vulkan includes and link libraries
target_include_directories(vulkantriangle PRIVATE 
    ${Vulkan_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/include"
)

target_link_libraries(vulkantriangle PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
)