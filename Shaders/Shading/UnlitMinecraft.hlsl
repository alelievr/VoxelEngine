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

	// i.uv are already in atlas format, transformation is made in the vertex shader
	o.color = float4(albedoMap.SampleLevel(nearestClamp, i.uv, 0).rgb, 1);

	// o.color = float4(frac(i.uv), 0, 1);

	return o;
}
