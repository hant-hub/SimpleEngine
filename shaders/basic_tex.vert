#version 450

layout(set = 0, binding = 0) uniform Buffer {
    vec3 colors[3];
} buf;

layout(location = 0) out vec2 texCoord;
layout(location = 0) in vec2 pos;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

vec2 texCoords[3] = vec2[](
        vec2(0.5f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 1.0f)
);

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    texCoord = texCoords[gl_VertexIndex];
}

