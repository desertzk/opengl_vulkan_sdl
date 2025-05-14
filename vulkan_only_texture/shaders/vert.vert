#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

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
    gl_Position = proj * view * model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
