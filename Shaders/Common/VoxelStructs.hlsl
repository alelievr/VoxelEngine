#ifndef VOXEL_STRUCTS
# define VOXEL_STRUCTS

struct	VoxelVertexInput
{
	[[vk::location(0)]] float3	position : POSITION;
	[[vk::location(1)]] float	atlasIndex : FOG;
};

struct	VoxelFragmentInput
{
	[[vk::location(0)]] float4	positionWS : SV_Position;
	[[vk::location(2)]] float2	uv : TEXCOORD0;
};

#endif // VOXEL_STRUCTS