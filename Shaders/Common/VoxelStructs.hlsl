#ifndef VOXEL_STRUCTS
# define VOXEL_STRUCTS

#define TOP_FACE		0
#define FORWARD_FACE	1
#define RIGHT_FACE		2

struct	VoxelVertexInput
{
	// TODO: float4 packing ?
	[[vk::location(0)]] float3	position : POSITION;
	[[vk::location(1)]] float	atlasIndex : FOG;
};

struct	VoxelFragmentInput
{
	[[vk::location(0)]] float4	positionWS : SV_Position;
	[[vk::location(1)]] float2	uv : TEXCOORD0;
};

// Note: must be the same as voxel vertex input
struct VoxelVertex
{
	// TODO: float4 packing ?
	float3 position;
	float data;
};

// function to pack/unpack VoxelVertexDatas
VoxelVertex	PackVoxelVertex(float3 posisiton, uint atlasIndex, uint faceIndex)
{
	VoxelVertex v;

	v.position = posisiton;
	v.data = asfloat(atlasIndex | (faceIndex << 8));

	return v;
}

void		UnpackVoxelVertex(VoxelVertex v, out float3 posisiton, out uint atlasIndex, out uint faceIndex)
{
	posisiton = v.position;
	atlasIndex = v.data & 0xFF;
	atlasIndex = v.data >> 8;
}

#endif // VOXEL_STRUCTS