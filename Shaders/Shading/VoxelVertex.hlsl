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

[vk::binding(0, 0)]
StructuredBuffer< SizeOffset >	sizeOffsets; // Metal 1 does not support Buffer<>
[vk::binding(1, 0)]
ConstantBuffer< Atlas >	atlas;

VoxelFragmentInput main(VoxelVertexInput i)
{
	VoxelFragmentInput	o;

	// Standard MVP transform, TODO: bake this into one matrix (and use a push constant to send it ?)
	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(i.position.xyz, 1), mvp);

	uint atlasIndex = uint(i.atlasIndex);
	float4 sizeOffset = sizeOffsets[atlasIndex].sizeOffset;

    sizeOffset.zw += atlas.size.zw * 0.5; // offset of half texel

	// TODO: vertex UV generation
    o.uv = UvToAtlas(float2(0, 0), sizeOffset);

	return o;
}
