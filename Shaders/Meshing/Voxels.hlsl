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

struct VertexCountReadback
{
	uint vertexCount;
};

RWStructuredBuffer< VertexCountReadback >	vertexCount;

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	float v = noiseVolume[i.dispatchThreadId];

	VoxelVertex v1 = {
		float3(0, 0, 0),
		0
	};

	VoxelVertex v2 = {
		float3(1, 0, 0),
		0
	};

	VoxelVertex v3 = {
		float3(1, 1, 0),
		0
	};

	VoxelVertex v4 = {
		float3(0, 1, 0),
		0
	};

	uint index;
	InterlockedAdd(vertexCount[0].vertexCount, 1, index);
	index *= 6;

	// Append a quad to test
	vertices[index + 0] = v1;
	vertices[index + 1] = v4;
	vertices[index + 2] = v3;
	vertices[index + 3] = v3;
	vertices[index + 4] = v2;
	vertices[index + 5] = v1;
}