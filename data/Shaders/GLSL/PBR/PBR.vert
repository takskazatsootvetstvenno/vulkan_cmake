//
#version 450
layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 texCoord_in;

layout(location = 0) out vec3 worldPos_out;
layout(location = 1) out vec3 norm_out;
layout(location = 2) out vec3 cameraPosition_out;
layout(location = 3) out vec2 texCoord_out;

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
	norm_out = normalize(localUBO.normalMatrix * normal_in); //(M^-1)^T
	vec4 locPos = localUBO.modelMatrix * vec4(position_in, 1.0);
	norm_out = normalize(transpose(inverse(mat3(localUBO.modelMatrix))) * normal_in);

	cameraPosition_out = globalUBO.cameraPosition;
	worldPos_out = locPos.xyz / locPos.w;
	texCoord_out = texCoord_in;

	gl_Position = globalUBO.projectionMatrix * globalUBO.viewMatrix * vec4(worldPos_out, 1.0);
}