#include "Common/UniformGraphic.hlsl"
#include "Common/Utils.hlsl"
#include "Common/VoxelStructs.hlsl"

struct FragmentOutput
{
	[[vk::location(0)]] float4	color : SV_Target0;
};

struct SizeOffset
{
	float4	sizeOffset;
};

struct Atlas
{
	float4	size;
};

FragmentOutput main(VoxelFragmentInput i)
{
	FragmentOutput	o;

	// o.color = float4(1, 1, 0, 1);

	// i.uv are already in atlas format, transformation is made in the vertex shader
	o.color = float4(albedoMap.SampleLevel(trilinearClamp, i.uv, 0).rgb, 0.5);

	return o;
}
