//
#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 texCoords_in;


layout(location = 0) out vec3 worldPos_out;
layout(location = 1) out vec3 norm_out;
layout(location = 2) out vec3 cameraPosition_out;
layout(location = 3) out vec2 texCoords_out;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 cameraPosition;
} globalUBO;

layout(set = 0, binding = 1) uniform MeshUbo
{
	mat4 modelMatrix;
	mat3 normalMatrix;
	vec4 baseColor;
	vec4 lightDirection;
	float metallic;
	float roughness;
} localUBO;

void main(){
	float a;
	norm_out = normalize(localUBO.normalMatrix * normal_in); //(M^-1)^T
	cameraPosition_out = globalUBO.cameraPosition;
	worldPos_out = vec3(localUBO.modelMatrix * vec4(position_in, 1.0));
	texCoords_out = texCoords_in;
	gl_Position = globalUBO.projectionMatrix * globalUBO.viewMatrix * localUBO.modelMatrix * vec4(position_in, 1.0);
}