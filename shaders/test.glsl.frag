#version 450

layout(location = 0) in vec3 vertColor;
layout(location = 0) out vec4 outColor;


void main() {
    //outColor = vec4(vec3(gl_FragCoord.z == 0), 1.0);
    outColor = vec4(vertColor, 1.0);
}

