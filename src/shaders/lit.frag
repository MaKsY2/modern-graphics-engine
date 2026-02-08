#version 330 core
in vec3 vNormalW;
in vec2 vUV;

uniform vec3 uLightDirW;

uniform vec4 uBaseColorFactor;
uniform sampler2D uBaseColorTex;
uniform bool uHasBaseColorTex;

out vec4 FragColor;

void main() {
  vec3 N = normalize(vNormalW);
  vec3 L = normalize(-uLightDirW);
  float diff = max(dot(N, L), 0.0);
  float ambient = 0.4;

  vec4 texCol = uHasBaseColorTex ? texture(uBaseColorTex, vUV) : vec4(1.0);
  vec4 base = uBaseColorFactor * texCol;

  vec3 lit = base.rgb * (ambient + (1.0 - ambient) * diff);
  FragColor = vec4(lit, base.a);
}
