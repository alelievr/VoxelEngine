#include "Shaders/Common/UniformGraphic.hlsl"
#include "Common/VoxelStructs.hlsl"
#include "Common/Utils.hlsl"

struct SizeOffset
{
	float4	sizeOffset;
};

struct Atlas
{
	float4	size;
};

[vk::binding(0, 4)]
StructuredBuffer< SizeOffset >	sizeOffsets; // Metal 1 does not support Buffer<>
[vk::binding(1, 4)]
ConstantBuffer< Atlas >	atlas;

static float2 UVs[] =
{
	float2(0, 0),
	float2(1, 0),
	float2(1, 1),
	float2(0, 1),
};

VoxelFragmentInput main(VoxelVertexInput i, uint vertexID : SV_VertexID)
{
	VoxelFragmentInput	o;

	// Standard MVP transform, TODO: bake this into one matrix (and use a push constant to send it ?)
	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(i.position.xyz, 1), mvp);

	uint atlasIndex = uint(i.atlasIndex);
	float4 sizeOffset = sizeOffsets[atlasIndex].sizeOffset;

    sizeOffset.zw += atlas.size.zw * 0.5; // offset of half texel

    // o.uv = UvToAtlas(float2(0, 0), sizeOffset);
	o.uv = i.position.xy % 2;

	return o;
}
