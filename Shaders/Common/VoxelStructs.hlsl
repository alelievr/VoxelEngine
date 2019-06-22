#ifndef VOXEL_STRUCTS
# define VOXEL_STRUCTS

#define TOP_FACE		0
#define FORWARD_FACE	1
#define RIGHT_FACE		2

#define VERTEX_BOTTOM_LEFT	0
#define VERTEX_BOTTOM_RIGHT	1
#define VERTEX_TOP_LEFT		2
#define VERTEX_TOP_RIGHT	3

#define VERTEX_IS_RIGHT(x)	((x & 0x1) != 0)
#define VERTEX_IS_BOTTOM(x) ((x & 0x2) != 0)

struct	VoxelVertexInput
{
	// TODO: float4 packing ?
	[[vk::location(0)]] float3	position : POSITION;
	[[vk::location(1)]] float	data : TEXCOORD0;
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

struct ChunkDescription
{
	uint3	chunkSize;
};

struct BlockData
{
	uint	topAtlasIndex;
	uint	sideAtlasIndex;
	uint	bottomAtlasIndex;
};

// function to pack/unpack VoxelVertexDatas
VoxelVertex	PackVoxelVertex(float3 posisiton, uint atlasIndex, uint faceIndex, uint vertexPosition)
{
	VoxelVertex v;

	v.position = posisiton;
	v.data = asfloat(atlasIndex | (faceIndex << 8) | (vertexPosition << 10));

	return v;
}

void		UnpackVoxelVertex(VoxelVertex v, out float3 posisiton, out uint atlasIndex, out uint faceIndex, out uint vertexPosition)
{
	posisiton = v.position;
	atlasIndex = asuint(v.data) & 0xFF;
	faceIndex = (asuint(v.data) >> 8) & 0x3; // 2 bits to store the faceIndex
	vertexPosition = (asuint(v.data) >> 10) & 0x3; // 2 bits to store the
}

#endif // VOXEL_STRUCTS