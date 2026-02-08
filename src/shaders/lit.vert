#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uViewProj;

out vec3 vNormalW;
out vec3 vPosW;
out vec2 vUV;

void main() {
  vec4 posW = uModel * vec4(aPos, 1.0);
  vPosW = posW.xyz;

  mat3 normalMat = mat3(transpose(inverse(uModel)));
  vNormalW = normalize(normalMat * aNormal);

  vUV = aUV;

  gl_Position = uViewProj * posW;
}
