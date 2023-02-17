//
#version 450

layout (location = 0) in vec2 texCoords_in;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D baseColorSampler;

void main() {
	outColor = texture(baseColorSampler, texCoords_in);
}