//
#version 450

layout (location = 0) in vec3 texCoords_in;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 8) uniform samplerCube skybox;

void main() {
	outColor = texture(skybox, texCoords_in);
}