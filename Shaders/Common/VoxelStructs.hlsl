#ifndef VOXEL_STRUCTS
# define VOXEL_STRUCTS

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
	float atlasIndex;
};

#endif // VOXEL_STRUCTS