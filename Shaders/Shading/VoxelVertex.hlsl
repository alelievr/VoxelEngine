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

void GenerateUVs(inout VoxelFragmentInput o, uint atlasIndex, uint faceIndex, uint vertexPosition)
{
	o.uv = float2(VERTEX_IS_RIGHT(vertexPosition), VERTEX_IS_BOTTOM(vertexPosition));

	float4 sizeOffset = sizeOffsets[atlasIndex].sizeOffset;
    o.uv = UvToAtlas(o.uv, sizeOffset);
}

VoxelFragmentInput main(VoxelVertexInput i)
{
	VoxelFragmentInput	o;
	float3				position;
	uint				atlasIndex;
	uint				faceIndex;
	uint				vertexPosition;

	VoxelVertex	v = {i.position, i.data};
	UnpackVoxelVertex(v, position, atlasIndex, faceIndex, vertexPosition);

	// Standard MVP transform, TODO: bake this into one matrix (and use a push constant to send it ?)
	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(position.xyz, 1), mvp);

	GenerateUVs(o, atlasIndex, faceIndex, vertexPosition);

	return o;
}
