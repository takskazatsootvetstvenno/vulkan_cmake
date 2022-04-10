#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 0) out vec3 pos_out;
layout(location = 1) out vec3 norm_out;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
} matrixUBO;

layout(set = 0, binding = 1) uniform GlobalUbo2
{
	mat4 modelMatrix;
	vec4 baseColor;
    vec4 lightDirection;
	float metallic;
	float roughness;
} localUBO;

void main(){
	norm_out = normalize(normal);
	pos_out = vec3(localUBO.modelMatrix * vec4(position, 1.0));
	gl_Position = matrixUBO.projectionMatrix * matrixUBO.viewMatrix * localUBO.modelMatrix * vec4(position, 1.0);
}