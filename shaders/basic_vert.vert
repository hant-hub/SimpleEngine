#version 450

layout(set = 0, binding = 0) uniform Buffer {
    vec3 colors[6];
} buf;

layout(location = 0) out vec3 fragColor;
layout(location = 0) in vec2 pos;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    float depth = float(int(gl_VertexIndex/3.0)) / 12.0;

    gl_Position = vec4(pos, depth, 1.0);
    fragColor = buf.colors[gl_VertexIndex] * (gl_VertexIndex/6.0);
}

