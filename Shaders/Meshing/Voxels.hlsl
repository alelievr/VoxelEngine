#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"
#include "Common/VoxelStructs.hlsl"

// TODO: something to manage auto-bindings from ComputeShader class
// [vk::binding(0, 1)]
// ConstantBuffer< LWGC_PerFrame > frame;

#define BACK_DOWN_LEFT float3(0, 0, 0)
#define BACK_DOWN_RIGHT float3(1, 0, 0)
#define BACK_TOP_RIGHT float3(1, 1, 0)
#define BACK_TOP_LEFT float3(0, 1, 0)
#define FORWARD_DOWN_LEFT float3(0, 0, 1)
#define FORWARD_DOWN_RIGHT float3(1, 0, 1)
#define FORWARD_TOP_RIGHT float3(1, 1, 1)
#define FORWARD_TOP_LEFT float3(0, 1, 1)

[vk::binding(0, 2)]
uniform Texture3D< half > noiseVolume;

[vk::binding(0, 3)]
RWStructuredBuffer< VoxelVertex > vertices;
// the field vertices_counter is bound to the index 0

struct DrawIndirectCommand
{
	uint vertexCount;
	uint instanceCount;
	uint firstVertex;
	uint firstInstance;
};

[vk::binding(0, 4)]
RWStructuredBuffer< DrawIndirectCommand >	drawCommands;

[vk::push_constant] cbuffer currentData { int targetDrawIndex; };

bool		IsAir(float value)
{
	return value <= 0;
}

bool		NeedsFace(float neighbourVoxelValue, bool centerIsAir, out bool invertFace)
{
	if (centerIsAir)
	{
		invertFace = true;
		return !IsAir(neighbourVoxelValue);
	}
	else
	{
		invertFace = false;
		return IsAir(neighbourVoxelValue);
	}
}

uint		GetBlockAtlasId(float3 p, float density)
{
	return (p.x % 3) ? uint((density + 1) * 63) : uint((density + 1) * 30);
}

void		AddFace(float3 v1, float3 v2, float3 v3, float3 v4, float3 offset, bool invert, uint face, uint atlasId)
{
	uint index;
	InterlockedAdd(drawCommands[targetDrawIndex].vertexCount, 6, index);

    float3 arr[] = {v1, v4, v3, v3, v2, v1};
    uint faces[] = {VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_RIGHT, VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT, VERTEX_TOP_LEFT, VERTEX_BOTTOM_LEFT};

    int start = 0;
    int iter = 1;
	if (invert)
    {
        start = 5;
        iter = -1;
    }

    for (uint i = start, j = 0; i <= 5 && i >= 0; i += iter, j++)
		vertices[index + i] = PackVoxelVertex(arr[j] + offset, atlasId, face, faces[j]);
}

groupshared float noiseVolumeCache[8*8*8];

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	// guard for samping outside (border)
	if (any(i.dispatchThreadId == 127)) // TODO: hardcoded chunk size
		return ;

#if 0 // USE_TGSM
	// We can win tow/three ALUs here by reordering the computation
	uint dispatchIndex = i.dispatchThreadId.x + i.dispatchThreadId.y * 8 + i.dispatchThreadId.z * 8 * 8;

	noiseVolumeCache[dispatchIndex] = noiseVolume[i.dispatchThreadId];

	GroupMemoryBarrierWithGroupSync();

	float right = noiseVolumeCache[dispatchIndex + 1];
	float top = noiseVolumeCache[dispatchIndex + 8];
	float forward = noiseVolumeCache[dispatchIndex + 8 * 8];
	float center = noiseVolumeCache[dispatchIndex];
#else
	float right = noiseVolume[i.dispatchThreadId + uint3(1, 0, 0)];
	float top = noiseVolume[i.dispatchThreadId + uint3(0, 1, 0)];
	float forward = noiseVolume[i.dispatchThreadId + uint3(0, 0, 1)];
	float center = noiseVolume[i.dispatchThreadId];
#endif

	bool centerIsAir = IsAir(center);

	bool invertTop;
	bool generateFaceTop = NeedsFace(top, centerIsAir, invertTop);
	bool invertRight;
	bool generateFaceRight = NeedsFace(right, centerIsAir, invertRight);
	bool invertForward;
	bool generateFaceForward = NeedsFace(forward, centerIsAir, invertForward);

	float3 offset = i.dispatchThreadId.xyz;

	// Check if Metal support this (technically it should)
	GroupMemoryBarrierWithGroupSync();

	uint atlasId = GetBlockAtlasId(i.dispatchThreadId, center);

	if (generateFaceTop)
		AddFace(BACK_TOP_LEFT, BACK_TOP_RIGHT, FORWARD_TOP_RIGHT, FORWARD_TOP_LEFT, offset, invertTop, TOP_FACE, atlasId);
	if (generateFaceForward)
		AddFace(FORWARD_TOP_RIGHT, FORWARD_DOWN_RIGHT, FORWARD_DOWN_LEFT, FORWARD_TOP_LEFT, offset, invertForward, FORWARD_FACE, atlasId);
	if (generateFaceRight)
		AddFace(BACK_TOP_RIGHT, BACK_DOWN_RIGHT, FORWARD_DOWN_RIGHT, FORWARD_TOP_RIGHT, offset, invertRight, RIGHT_FACE, atlasId);
}