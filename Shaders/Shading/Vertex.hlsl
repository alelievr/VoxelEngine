#include "Shaders/Common/UniformGraphic.hlsl"
#include "Common/VoxelStructs.hlsl"

VoxelFragmentInput main(VoxelVertexInput i)
{
	VoxelFragmentInput	o;

	float4x4 mvp = camera.projection * camera.view * object.model;
	o.positionWS = mul(float4(i.position.xyz, 1), mvp);
	uint atlasIndex = uint(i.atlasIndex);

	// TODO: generate UVs from the texture atlas coordinates
    o.uv = float2(0, 0);

	return o;
}
