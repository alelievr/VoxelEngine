#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"
#include "Common/VoxelStructs.hlsl"

// TODO: something to manage auto-bindings from ComputeShader class
// [vk::binding(0, 1)]
// ConstantBuffer< LWGC_PerFrame > frame;

[vk::binding(0, 2)]
uniform Texture3D< half > noiseVolume;

[vk::binding(0, 3)]
AppendStructuredBuffer< VoxelVertex > vertices;
// the field vertices_counter is bound to the index 0

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

	// Append a quad to test
	vertices.Append(v1);
	// vertices.Append(v2);
	// vertices.Append(v3);
	// vertices.Append(v3);
	// vertices.Append(v4);
	// vertices.Append(v1);
}