cmake_minimum_required(VERSION 3.10)
project(vulkan_video_check)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(vulkan_video_check vulkan_video_check.cpp)

target_link_libraries(vulkan_video_check PRIVATE Vulkan::Vulkan)
