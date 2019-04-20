#include "Shaders/Common/UniformGraphic.hlsl"

struct	VertexInput
{
	[[vk::location(0)]] float3	position : POSITION;
	[[vk::location(1)]] float	atlasIndex : NORMAL;
};

struct	FragmentInput
{
	[[vk::location(0)]] float4	positionWS : SV_Position;
	[[vk::location(1)]] float3	normalOS : NORMAL;
	[[vk::location(2)]] float2	uv : TEXCOORD0;
};

FragmentInput main(VertexInput i, int id : SV_VertexID, int elementID : SV_InstanceID)
{
	FragmentInput	o;

    o.uv = i.uv;
	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(i.position.xyz, 1), mvp);
	o.normalOS = i.normal;

	return o;
}
