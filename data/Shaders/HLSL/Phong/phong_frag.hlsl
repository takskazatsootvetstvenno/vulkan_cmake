//
static const float M_PI = 3.141592653589793;
static const float3 lightPoint = float3(1.f, -2.f, 1.f);
static const float gamma = 2.2f;

struct VS_INPUT
{
	float4 ObjectPosition : SV_Position;
	float3 ObjectNormal : NORMAL;
	float3 CameraPosition : POSITION1;
	float3 worldPos : POSITION2;
	float2 texCoords : TEXCOORD;
};

[[vk::binding(1, 0)]] cbuffer CBufferPerObject
{
	row_major float4x4 Model : WORLD;
	row_major float3x3 NormalModel : WORLDNORMAL;
	float4 baseColor;
}

float4 main( VS_INPUT IN ) : SV_Target
{
	const float3 L = normalize(lightPoint - IN.worldPos);
	const float3 V = normalize(IN.CameraPosition - IN.worldPos);
	const float3 N = normalize(IN.ObjectNormal);
     	
	float diffuse = max(dot(N, L), 0.f);
	float3 reflectDir = reflect(-L, N);
	float spec = pow(max(dot(V, reflectDir), 0.0), 32);
	const float R = length(lightPoint - IN.worldPos);

	float3 Color = ((0.1f + diffuse + spec) / (R*R) * baseColor).xyz;
	return float4(pow(Color, float3(1.f/gamma, 1.f / gamma, 1.f / gamma)), 1.f);
}