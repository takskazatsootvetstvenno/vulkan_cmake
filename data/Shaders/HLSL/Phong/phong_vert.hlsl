[[vk::binding(0, 0)]] cbuffer CBufferPerObject
{
	row_major float4x4 Projection : PROJECTION;
	row_major float4x4 View : VIEW;
	float3 CameraPosition;
}

[[vk::binding(1, 0)]] cbuffer CBufferPerObject
{
	row_major float4x4 Model : WORLD;
	row_major float3x3 NormalMatrix : WORLDNORMAL;
}

struct VS_INPUT
{
	float3 ObjectPosition : POSITION;
	float3 ObjectNormal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float3 CameraPosition :POSITION1;
	float3 world_pos :POSITION2;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 nop = float4(IN.ObjectPosition, 1.0);

	OUT.Position = mul(mul(mul(nop, Model), View), Projection);
   	OUT.Normal = normalize(mul(IN.ObjectNormal, NormalMatrix));
	OUT.CameraPosition = CameraPosition;
	OUT.world_pos = (float3)mul(nop,Model);
	return OUT;
}
