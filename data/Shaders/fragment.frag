#version 450
layout (location = 0) in vec3 worldPos_in;
layout (location = 1) in vec3 norm_in;
layout (location = 2) in vec3 cameraPosition_in;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform MeshUbo
{
	mat4 modelMatrix;
	vec4 baseColor;
    vec4 lightDirection;
	float metallic;
	float roughness;
} localUBO;

const float M_PI = 3.141592653589793;
const vec3 lightPoint = vec3(1.f,-2.f,1.f);

vec3 fDiffuse(const vec3 surfaceColor)
{
	return surfaceColor/M_PI;
}

float geometricOcclusion(const float NdotL, const float NdotV, const float alpha)	//G
{
	const float attenuationL = 2.0 * NdotL / (NdotL + sqrt(alpha * alpha + (1.0 - alpha * alpha) * (NdotL * NdotL)));
	const float attenuationV = 2.0 * NdotV / (NdotV + sqrt(alpha * alpha + (1.0 - alpha * alpha) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

vec3 specularFresnell(const vec3 specEnvR0, const vec3 specEnvR90, const float VdotH)  //F
{
	return specEnvR0 + (specEnvR90 - specEnvR0) * pow(clamp(1.f - VdotH, 0.f, 1.f), 5.f);
}

float microfacetDistribution(const float alpha, const float NdotH) //D
{
	const float alphaSq = alpha * alpha;
	const float f = (NdotH * alphaSq - NdotH) * NdotH + 1;
	return alphaSq/(M_PI * f * f);
}

vec3 getPointLightColor()
{
	const float denominator = distance(lightPoint, worldPos_in);
	return vec3(1.f)/(denominator*denominator);
}

void main() {

	const vec3 L = normalize(lightPoint - worldPos_in); //normalize(localUBO.lightDirection.xyz);
	const vec3 V = normalize(cameraPosition_in - worldPos_in);
	const vec3 N = normalize(norm_in);
	const vec3 H = normalize(V + L);

	const float NdotL = clamp(dot(N, L), 0.001, 1.0);
	const float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
	const float NdotH = clamp(dot(N, H), 0.0, 1.0);
	const float LdotH = clamp(dot(L, H), 0.0, 1.0);
	const float VdotH = clamp(dot(V, H), 0.0, 1.0);
	
	vec3 f0 = vec3(0.04);
	float metallic = localUBO.metallic;
	vec3 specularColor = mix(f0, localUBO.baseColor.rgb, metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	const float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	const vec3 specEnvR0 = specularColor.rgb;
	const vec3 specEnvR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	const float alpha = localUBO.roughness * localUBO.roughness;


	const float D = microfacetDistribution(alpha, NdotH);
	const float G = geometricOcclusion(NdotL, NdotV, alpha);
	const vec3 F = specularFresnell(specEnvR0, specEnvR90, VdotH);

	const vec3 u_LightColor = vec3(1.0);
	vec3 diffuseColor = localUBO.baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	const vec3 diffuseContrib = (1.0 - F) * fDiffuse(diffuseColor);
	const vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	const vec3 color = (diffuseContrib + specContrib) * getPointLightColor() * NdotL;   
	//vec3 color = (diffuseContrib + specContrib) * u_LightColor * NdotL;   
	// color += IBL(...);
	/*
	float diffuse = max(dot(N, L), 0.f);

	vec3 reflectDir = reflect(-L, N);  
	float spec = pow(max(dot(V, reflectDir), 0.0), 32);

	outColor = (0.1f + diffuse + spec) * localUBO.baseColor;
	*/
	outColor = vec4(color, 1.f);
}