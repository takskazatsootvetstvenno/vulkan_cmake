#version 450
layout (location = 0) in vec3 worldPos_in;
layout (location = 1) in vec3 norm_in;
layout (location = 2) in vec3 cameraPosition_in;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform MeshUbo
{
	mat4 modelMatrix;
	mat3 normalMatrix;
	vec4 baseColor;
    vec4 lightDirection;
	float metallic;
	float roughness;
} localUBO;

const float M_PI = 3.141592653589793;
const vec3 lightPoint = vec3(1.f,-2.f,1.f);
const float gamma = 2.2f;

void main() {

	const vec3 L = normalize(lightPoint - worldPos_in); //normalize(localUBO.lightDirection.xyz);
	const vec3 V = normalize(cameraPosition_in - worldPos_in);
	const vec3 N = normalize(norm_in);
     	
	float diffuse = max(dot(N, L), 0.f);

	vec3 reflectDir = reflect(-L, N);  
	float spec = pow(max(dot(V, reflectDir), 0.0), 32);
	const float R = length(lightPoint - worldPos_in);

	vec3 Color = ((0.1f + diffuse + spec) / (R*R) * localUBO.baseColor).xyz;
	outColor = vec4(pow(Color, vec3(1.f/gamma)), 1.f);
	//outColor = vec4(1.f,0.5f,0.5f,1.f);
}