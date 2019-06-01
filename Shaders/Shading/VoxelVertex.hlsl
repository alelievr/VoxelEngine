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

void GenerateUVs(inout VoxelFragmentInput o, float3 position, uint atlasIndex, uint faceIndex)
{
	// Almost free switch, no code divergence :)
	switch (faceIndex)
	{
		case TOP_FACE:
			o.uv = position.xz % 2;
			break ;
		case FORWARD_FACE:
			o.uv = position.xy % 2;
			// o.uv = float2(1, 1);
			break ;
		default: // RIGHT_FACE
			o.uv = position.yz % 2;
			// o.uv = position.xz % 1;
			break ;
	}
	
	float4 sizeOffset = sizeOffsets[atlasIndex].sizeOffset;
    sizeOffset.zw += atlas.size.zw * 0.5; // offset of half texel
    o.uv = UvToAtlas(o.uv, sizeOffset); // TODO
}

VoxelFragmentInput main(VoxelVertexInput i)
{
	VoxelFragmentInput	o;
	float3				position;
	uint				atlasIndex;
	uint				faceIndex;

	VoxelVertex	v = {i.position, i.data};
	UnpackVoxelVertex(v, position, atlasIndex, faceIndex);

	// Standard MVP transform, TODO: bake this into one matrix (and use a push constant to send it ?)
	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(position.xyz, 1), mvp);

	GenerateUVs(o, position, atlasIndex, faceIndex);

	return o;
}
