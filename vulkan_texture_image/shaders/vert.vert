#version 450

// location 0: vec2 position; location 1: vec2 texCoord
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // Model matrix (identity)
    mat4 model = mat4(1.0);

    // View matrix (camera at (0,0,1), looking at (0,0,0), up = (0,-1,0))
    mat4 view = mat4(
        -1.0, 0.0,  0.0, 0.0,  // Column 0
         0.0, -1.0, 0.0, 0.0,  // Column 1
         0.0, 0.0, 1.0, -1.0,  // Column 2 (translation z = 1)
         0.0, 0.0, 0.0, 1.0    // Column 3
    );

    // Orthographic projection with Y-flip (covers [-1, 1] in x/y)
    mat4 proj = mat4(
        1.0, 0.0, 0.0, 0.0,   // Column 0
        0.0, -1.0, 0.0, 0.0,  // Column 1 (Y-flip for Vulkan)
        0.0, 0.0, -1.0, 0.0,   // Column 2
        0.0, 0.0, 0.0, 1.0     // Column 3
    );

    // Apply transformations: proj * view * model * position
    gl_Position    = proj * view * model * vec4(inPos, 0.0, 1.0);
    fragTexCoord   = inTex;
}
