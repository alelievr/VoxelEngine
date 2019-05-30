#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"
#include "Common/VoxelStructs.hlsl"

// TODO: something to manage auto-bindings from ComputeShader class
[vk::binding(0, 1)]
ConstantBuffer< LWGC_PerFrame > frame;

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
	return value < frame.time.y;
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

void		AddFace(VoxelVertex v1, VoxelVertex v2, VoxelVertex v3, VoxelVertex v4, bool invert)
{
	uint index;
	InterlockedAdd(drawCommands[targetDrawIndex].vertexCount, 6, index);

	// Append a quad to test
	if (invert)
	{
		vertices[index + 5] = v1;
		vertices[index + 4] = v4;
		vertices[index + 3] = v3;
		vertices[index + 2] = v3;
		vertices[index + 1] = v2;
		vertices[index + 0] = v1;
	}
	else
	{
		vertices[index + 0] = v1;
		vertices[index + 1] = v4;
		vertices[index + 2] = v3;
		vertices[index + 3] = v3;
		vertices[index + 4] = v2;
		vertices[index + 5] = v1;
	}
}

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	// guard for samping outside (border)
	if (any(i.dispatchThreadId == 128)) // TODO: hardcoded chunk size
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
	VoxelVertex v1 = {
		float3(0, 0, 0) + offset,
		0
	};

	// back back right
	VoxelVertex v2 = {
		float3(1, 0, 0) + offset,
		0
	};

	// back top right
	VoxelVertex v3 = {
		float3(1, 1, 0) + offset,
		0
	};

	// back top left
	VoxelVertex v4 = {
		float3(0, 1, 0) + offset,
		0
	};

	// forward back left
	VoxelVertex v5 = {
		float3(0, 0, 1) + offset,
		0
	};

	// forward back right
	VoxelVertex v6 = {
		float3(1, 0, 1) + offset,
		0
	};

	// forward top right
	VoxelVertex v7 = {
		float3(1, 1, 1) + offset,
		0
	};

	// forward top left
	VoxelVertex v8 = {
		float3(0, 1, 1) + offset,
		0
	};

	if (generateFaceTop)
		AddFace(v4, v3, v7, v8, invertTop);
	if (generateFaceForward)
		AddFace(v8, v7, v6, v5, invertForward);
	if (generateFaceRight)
		AddFace(v3, v2, v6, v7, invertRight);
}