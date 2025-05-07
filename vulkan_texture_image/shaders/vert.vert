#version 450

// location 0: vec2 position; location 1: vec2 texCoord
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    gl_Position    = vec4(inPos, 0.0, 1.0);
    fragTexCoord   = inTex;
}
