//
static const float M_PI = 3.141592653589793;
static const float3 lightPoint = float3(1.f, -2.f, 1.f);
static const float gamma = 2.2f;

struct VS_INPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float3 CameraPosition :POSITION1;
	float3 worldPos :POSITION2;
	float2 TexCoords : TEXCOORD;
};

[[vk::binding(1, 0)]] cbuffer CBufferPerObject
{
	row_major float4x4 Model : WORLD;
	row_major float3x3 NormalModel : WORLDNORMAL;
	float4 baseColor;
	float4 lightDirection;
	float metallic;
	float roughness;
} 

float3 fDiffuse(const float3 surfaceColor)
{
	return surfaceColor / M_PI;
}

float geometricOcclusion(const float NdotL, const float NdotV, const float alpha)	//G
{
	const float attenuationL = 2.0 * NdotL / (NdotL + sqrt(alpha * alpha + (1.0 - alpha * alpha) * (NdotL * NdotL)));
	const float attenuationV = 2.0 * NdotV / (NdotV + sqrt(alpha * alpha + (1.0 - alpha * alpha) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

float3 specularFresnell(const float3 specEnvR0, const float3 specEnvR90, const float VdotH)  //F
{
	return specEnvR0 + (specEnvR90 - specEnvR0) * pow(clamp(1.f - VdotH, 0.f, 1.f), 5.f);
}

float microfacetDistribution(const float alpha, const float NdotH) //D
{
	const float alphaSq = alpha * alpha;
	const float f = (NdotH * alphaSq - NdotH) * NdotH + 1;
	return alphaSq / (M_PI * f * f);
}

float3 getPointLightColor(const float3 worldPosition)
{
	const float denominator = distance(lightPoint, worldPosition);
	return float3(1.f, 1.f, 1.f) / (denominator * denominator);
}

float4 main( VS_INPUT IN ) : SV_Target
{
	const float3 L = normalize(lightPoint - IN.worldPos);
	const float3 V = normalize(IN.CameraPosition - IN.worldPos);
	const float3 N = normalize(IN.Normal);
	const float3 H = normalize(V + L);

	const float NdotL = clamp(dot(N, L), 0.001, 1.0);
	const float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
	const float NdotH = clamp(dot(N, H), 0.0, 1.0);
	const float LdotH = clamp(dot(L, H), 0.0, 1.0);
	const float VdotH = clamp(dot(V, H), 0.0, 1.0);

	float3 f0 = float3(0.04f, 0.04f, 0.04f);
	float mesh_metallic = metallic;
	float3 specularColor = lerp(f0, baseColor.rgb, mesh_metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	const float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	const float3 specEnvR0 = specularColor.rgb;
	const float3 specEnvR90 = float3(1.0, 1.0, 1.0) * reflectance90;

	const float alpha = roughness * roughness;


	const float D = microfacetDistribution(alpha, NdotH);
	const float G = geometricOcclusion(NdotL, NdotV, alpha);
	const float3 F = specularFresnell(specEnvR0, specEnvR90, VdotH);

	const float3 u_LightColor = float3(1.f, 1.f, 1.f);
	float3 diffuseColor = baseColor.rgb * (float3(1.f, 1.f, 1.f) - f0);
	diffuseColor *= 1.0 - mesh_metallic;

	const float3 diffuseContrib = (1.0 - F) * fDiffuse(diffuseColor);
	const float3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	const float3 color = (diffuseContrib + specContrib) * getPointLightColor(IN.worldPos) * NdotL;
	return float4(pow(color, float3(1.f / gamma, 1.f / gamma, 1.f / gamma)), 1.f);
}