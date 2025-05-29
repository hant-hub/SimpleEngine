#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inColor;

layout(location = 0) out vec3 outColor;

vec3 pos[] = {
    {0.0, -0.5, 0.5f},
    {0.5, 0.5, 0.5f}, 
    {-0.5, 0.5, 0.5f},
};

void main() {
    //gl_Position = vec4(inPosition, 1.0);
    gl_Position = vec4(pos[gl_VertexIndex], 1.0);
    //outColor = vec3(inPosition.z);
    outColor = vec3(0.0);
}

