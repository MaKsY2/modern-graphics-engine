#version 450
layout(location = 0) in vec3 inPos;
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform MVP {
    mat4 mvp;
} ubo;

void main() {
    gl_Position = ubo.mvp * vec4(inPos, 1.0);
    fragColor = vec3(1,1,0); // жёлтый
}
