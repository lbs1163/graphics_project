#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D cover;
uniform bool isBlack;

void main() {
	vec4 texColor = isBlack ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : texture(cover, TexCoords);
	FragColor = texColor;
}