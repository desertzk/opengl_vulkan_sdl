#version 450

layout(location = 0) out vec2 uv;

void main() {
    vec2 positions[3] = vec2[](
        vec2(-1, -1),
        vec2(3, -1),
        vec2(-1, 3)
    );
    vec2 uvs[3] = vec2[](
        vec2(0, 1),
        vec2(2, 1),
        vec2(0, -1)
    );
    
    gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
    uv = uvs[gl_VertexIndex];
}