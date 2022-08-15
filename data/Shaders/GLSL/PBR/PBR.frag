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
layout(set = 0, binding = 100) uniform DebugUBO
{
	uint outType;
} debug;

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

layout(set = 0, binding = 8) uniform samplerCube skybox;
layout(set = 0, binding = 9) uniform samplerCube irradiance;
layout(set = 0, binding = 10) uniform sampler2D brdfLUT;
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
	vec3 normalMap = texture(NormalSampler, texCoords_in).xyz * 2.0 - 1.0;
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
    
	/*
	vec3 q1 = dFdx(worldPos_in);
	vec3 q2 = dFdy(worldPos_in);
	vec2 st1 = dFdx(texCoords_in);
	vec2 st2 = dFdy(texCoords_in);

	vec3 N = normalize(norm_in);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	*/
#ifdef HAS_NORMAL_MAP	
	vec3 n = normalize(TBN * normalMap);
#else
	vec3 n = N;
#endif
	return n;
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
    vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);;
}

vec3 getIBLContribution(vec3 diffuseColor, vec3 specularColor, vec3 n, vec3 reflection, float NdotV, float roughness ){
	vec3 diffuse = SRGBtoLINEAR(texture(irradiance, -n)).rgb;

	vec3 brdf = texture(brdfLUT, vec2(NdotV, 1.0 - roughness)).rgb;

	vec3 specular = SRGBtoLINEAR(texture(skybox, reflection)).rgb;

	diffuse *= diffuseColor;
	specular *= (specularColor * brdf.x + brdf.y);
	return diffuse+specular;
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

	roughness = clamp(roughness, 0.04, 1.0);
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
	vec3 reflection = -normalize(reflect(V, N));

	color += getIBLContribution(diffuseColor,specularColor, N, reflection, NdotV, roughness);
	//vec3 reflection = -normalize(reflect(V, N));

	//color += 0.5f*vec3(texture(CubemapSampler, N));

#ifdef HAS_EMISSIVE_MAP
	vec3 emissive = texture(EmissiveSampler, texCoords_in).rgb;
	color += emissive;
#endif
	
	outColor = vec4(pow(color, vec3(1.f/gamma)), 1.f);

	switch(debug.outType)
	{
	case 0:
		outColor = vec4(pow(color, vec3(1.f/gamma)), 1.f);
		break;
	case 1:
		outColor = (vec4(getNormal(), 1.f) + 1.f) / 2.f;
		break;
	case 2:
#ifdef HAS_NORMAL_MAP
		outColor = vec4(texture(NormalSampler, texCoords_in).xyz, 1.f);
#endif
		break;
	case 3:
		outColor = (vec4(texCoords_in, 0.f, 0.f) + 1.f) /2.f;		
		break;
	case 4:
		outColor = (vec4(normalize(norm_in), 1.f) + 1.f) / 2.f;
		break;
	case 5:
		outColor = (vec4(normalize(worldPos_in), 1.f) + 1.f) / 2.f;
		break;
	}
}