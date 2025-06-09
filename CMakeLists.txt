cmake_minimum_required(VERSION 3.15)
project(VULKAN_SDL)

# Add SDL3 from the vendor directory
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
# Add the GLFW subdirectory from our vendored folder
add_subdirectory(${CMAKE_SOURCE_DIR}/vendored/glfw EXCLUDE_FROM_ALL)

# Add the assimp subdirectory from our vendored folder
add_subdirectory(${CMAKE_SOURCE_DIR}/vendored/assimp EXCLUDE_FROM_ALL)

# Add the assimp subdirectory from our vendored folder
add_subdirectory(${CMAKE_SOURCE_DIR}/vendored/gli EXCLUDE_FROM_ALL)

# Add the v4l2_vulkan_video project
add_subdirectory(v4l2_vulkan_video)

# Add the triangle project (unchanged)
add_subdirectory(triangle)

add_subdirectory(triangle_glfw)

# Add the v4l2_sdl_video project
add_subdirectory(v4l2_sdl_video)


add_subdirectory(vulkan_vertex_index_buffer)

add_subdirectory(vulkan_only_texture)

add_subdirectory(vulkan_texture_image)

add_subdirectory(vulkan_texture_image0)

add_subdirectory(vulkan_texture_image1)

add_subdirectory(vulkan_texture_image_class)

add_subdirectory(opengl_triangle)

add_subdirectory(opengl_camera_glfw)

add_subdirectory(opengl_model_loading)

#add_subdirectory(opengl_fbxmodel_loading)