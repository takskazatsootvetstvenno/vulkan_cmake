//
#version 450

layout (location = 0) in vec3 worldPos_in;
layout (location = 1) in vec3 norm_in;
layout (location = 2) in vec3 cameraPosition_in;
layout (location = 3) in vec2 texCoords_in;

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
const vec3 lightPoint = vec3(1.f, -2.f, 1.f);
const float gamma = 2.2f;

#ifdef HAS_COLOR_MAP
	layout(set = 0, binding = 2) uniform sampler2D baseColorSampler;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_MAP
	layout(set = 0, binding = 3) uniform sampler2D MRSampler;
#endif

#ifdef HAS_NORMAL_MAP
	layout(set = 0, binding = 4) uniform sampler2D NormalSampler;
#endif

#ifdef HAS_EMISSIVE_MAP
	layout(set = 0, binding = 5) uniform sampler2D EmissiveSampler;
#endif
	layout(set = 0, binding = 5) uniform samplerCube CubemapSampler;

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

vec3 getNormal()
{
#ifdef HAS_NORMAL_MAP
	vec3 normalMAP = texture(baseColorSampler, texCoords_in).rgb * 2.f - 1.f;
#endif
	const vec3 N = normalize(norm_in);
	const vec3 V = normalize(worldPos_in);
	vec3 dp1 = dFdx(-V);
    vec3 dp2 = dFdy(-V);
    vec2 duv1 = dFdx(texCoords_in);
    vec2 duv2 = dFdy(texCoords_in);

  // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame 
  float invmax = 1.0 / sqrt(max(dot(T,T), dot(B,B)));
  mat3 TBN = mat3(T * invmax, B * invmax, N);
#ifdef HAS_NORMALMAP	
	vec3 n  = normalize(TBN * normalMap);
#else
	vec3 n = N;
#endif
	return n;
}
void main() {

	const vec3 L = normalize(lightPoint - worldPos_in); //normalize(localUBO.lightDirection.xyz);
	const vec3 V = normalize(cameraPosition_in - worldPos_in);

	const vec3 N = getNormal();
	const vec3 H = normalize(V + L);

	const float NdotL = clamp(dot(N, L), 0.001, 1.0);
	const float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
	const float NdotH = clamp(dot(N, H), 0.0, 1.0);
	const float LdotH = clamp(dot(L, H), 0.0, 1.0);
	const float VdotH = clamp(dot(V, H), 0.0, 1.0);
	
#ifdef HAS_COLOR_MAP
	vec4 basecolor = localUBO.baseColor * texture(baseColorSampler, texCoords_in);
#else
	vec4 basecolor = localUBO.baseColor;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_MAP
	vec3 value = texture(MRSampler, texCoords_in).rgb;
	float metallic = value.g;
	float roughness = value.b;
#else
	float metallic = localUBO.metallic;
	float roughness = localUBO.roughness;
#endif


	vec3 f0 = vec3(0.04);
	vec3 specularColor = mix(f0, basecolor.rgb, metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	const float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	const vec3 specEnvR0 = specularColor.rgb;
	const vec3 specEnvR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	const float alpha = roughness * roughness;

	const float D = microfacetDistribution(alpha, NdotH);
	const float G = geometricOcclusion(NdotL, NdotV, alpha);
	const vec3 F = specularFresnell(specEnvR0, specEnvR90, VdotH);

	const vec3 u_LightColor = vec3(1.0);

	vec3 diffuseColor = basecolor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;

	const vec3 diffuseContrib = (1.0 - F) * fDiffuse(diffuseColor);
	const vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	//vec3 color = (diffuseContrib + specContrib) * getPointLightColor() * NdotL;   
	vec3 color = (diffuseContrib + specContrib) * vec3(1.f) * NdotL;
	
#ifdef HAS_EMISSIVE_MAP
	vec3 emissive = texture(EmissiveSampler, texCoords_in).rgb;
	color += emissive;
#endif
	outColor = vec4(pow(color, vec3(1.f/gamma)), 1.f);
}