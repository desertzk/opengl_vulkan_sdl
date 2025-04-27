# vulkan_sdl

git clone https://github.com/libsdl-org/SDL.git vendored/SDL

cmake -S . -B build

glslc vert.vert -o vert.spv
glslc frag.frag -o frag.spv
