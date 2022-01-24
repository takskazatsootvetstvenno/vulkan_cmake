#version 450
layout (location = 0) in vec2 in_pos;
layout (location = 0) out vec4 outColor;

void main() {
	outColor = vec4(in_pos + vec2(0.5,0.5), 1.0, 1.0);
}