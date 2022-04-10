#version 450
layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 norm_in;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform GlobalUbo2
{
	mat4 modelMatrix;
	vec4 baseColor;
    vec4 lightDirection;
	float metallic;
	float roughness;
} localUBO;

void main() {
	vec3 lighDir = normalize(localUBO.lightDirection.xyz);
	vec3 Norm = normalize(norm_in);
	float power = max(dot(Norm, lighDir), 0.f);
	outColor = (0.1f + power) * localUBO.baseColor;// vec4(localUBO.metallic, localUBO.roughness, 0.f, 1.0);
}