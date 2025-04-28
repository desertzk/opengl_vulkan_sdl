cmake_minimum_required(VERSION 3.15)
project(v4l2_sdl3_capture_playback VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${V4L2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SDL3::SDL3          # SDL3 library target
    ${V4L2_LIBRARIES}  # V4L2 (e.g. -lv4l2)
)
