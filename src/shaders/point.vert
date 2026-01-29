#version 330 core
layout(location = 0) in vec3 inPos;

uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(inPos, 1.0);
    gl_PointSize = 12.0; // размер суставов/таргета
}
