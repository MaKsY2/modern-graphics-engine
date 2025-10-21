#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

void main()
{
	FragColor = vec4(TexCoord.x, TexCoord.y, 0.5, 1.0);
}