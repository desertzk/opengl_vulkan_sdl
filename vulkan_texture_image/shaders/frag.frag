#version 450

layout(location = 0) in  vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

// binding = 0 must match your descriptor set layout
layout(binding = 0) uniform sampler2D texSampler;

void main() {
    // Sample the RGBA texture
    outColor = texture(texSampler, fragTexCoord);
}
