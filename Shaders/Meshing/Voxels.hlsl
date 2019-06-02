#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"
#include "Common/VoxelStructs.hlsl"

// TODO: something to manage auto-bindings from ComputeShader class
// [vk::binding(0, 1)]
// ConstantBuffer< LWGC_PerFrame > frame;

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

void		AddFace(float3 v1, float3 v2, float3 v3, float3 v4, bool invert, uint face)
{
	uint index;
	InterlockedAdd(drawCommands[targetDrawIndex].vertexCount, 6, index);

	uint atlasId = 18;

	// Append a quad to test
	if (invert)
	{
		vertices[index + 5] = PackVoxelVertex(v1, atlasId, face, VERTEX_BOTTOM_LEFT);
		vertices[index + 4] = PackVoxelVertex(v4, atlasId, face, VERTEX_BOTTOM_RIGHT);
		vertices[index + 3] = PackVoxelVertex(v3, atlasId, face, VERTEX_TOP_RIGHT);
		vertices[index + 2] = PackVoxelVertex(v3, atlasId, face, VERTEX_TOP_RIGHT);
		vertices[index + 1] = PackVoxelVertex(v2, atlasId, face, VERTEX_TOP_LEFT);
		vertices[index + 0] = PackVoxelVertex(v1, atlasId, face, VERTEX_BOTTOM_LEFT);
	}
	else
	{
		vertices[index + 0] = PackVoxelVertex(v1, atlasId, face, VERTEX_BOTTOM_LEFT);
		vertices[index + 1] = PackVoxelVertex(v4, atlasId, face, VERTEX_BOTTOM_RIGHT);
		vertices[index + 2] = PackVoxelVertex(v3, atlasId, face, VERTEX_TOP_RIGHT);
		vertices[index + 3] = PackVoxelVertex(v3, atlasId, face, VERTEX_TOP_RIGHT);
		vertices[index + 4] = PackVoxelVertex(v2, atlasId, face, VERTEX_TOP_LEFT);
		vertices[index + 5] = PackVoxelVertex(v1, atlasId, face, VERTEX_BOTTOM_LEFT);
	}
}

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	// guard for samping outside (border)
	if (any(i.dispatchThreadId == 127)) // TODO: hardcoded chunk size
		return ;

	float center = noiseVolume[i.dispatchThreadId];

	bool centerIsAir = IsAir(center);

	float right = noiseVolume[i.dispatchThreadId + uint3(1, 0, 0)];
	float top = noiseVolume[i.dispatchThreadId + uint3(0, 1, 0)];
	float forward = noiseVolume[i.dispatchThreadId + uint3(0, 0, 1)];

	bool invertTop;
	bool generateFaceTop = NeedsFace(top, centerIsAir, invertTop);
	bool invertRight;
	bool generateFaceRight = NeedsFace(right, centerIsAir, invertRight);
	bool invertForward;
	bool generateFaceForward = NeedsFace(forward, centerIsAir, invertForward);

	float3 offset = i.dispatchThreadId.xyz;

	// Check if Metal support this (technically it should)
	GroupMemoryBarrierWithGroupSync();

	// TODO: refactor this: way too much VGPR used

	// back back left
	float3 v1 = float3(0, 0, 0) + offset;

	// back back right
	float3 v2 = float3(1, 0, 0) + offset;

	// back top right
	float3 v3 = float3(1, 1, 0) + offset;

	// back top left
	float3 v4 = float3(0, 1, 0) + offset;

	// forward back left
	float3 v5 = float3(0, 0, 1) + offset;

	// forward back right
	float3 v6 = float3(1, 0, 1) + offset;

	// forward top right
	float3 v7 = float3(1, 1, 1) + offset;

	// forward top left
	float3 v8 = float3(0, 1, 1) + offset;

	if (generateFaceTop)
		AddFace(v4, v3, v7, v8, invertTop, TOP_FACE);
	if (generateFaceForward)
		AddFace(v7, v6, v5, v8, invertForward, FORWARD_FACE);
	if (generateFaceRight)
		AddFace(v3, v2, v6, v7, invertRight, RIGHT_FACE);
}