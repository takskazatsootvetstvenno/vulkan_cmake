//
#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

layout(location = 0) out VS_OUT {
    vec3 normal;
} vs_out;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 cameraPosition;
} globalUBO;

layout(set = 0, binding = 1) uniform NormalUboInfo
{
    mat4 model;
	float magnitude;
} normalTestUBO;

void main(){
	mat3 normalMatrix = mat3(transpose(inverse(globalUBO.viewMatrix * normalTestUBO.model)));
	vs_out.normal = normalize(normalMatrix * normal_in); //(M^-1)^T
	gl_Position = globalUBO.viewMatrix * normalTestUBO.model * vec4(position_in, 1.0);
}