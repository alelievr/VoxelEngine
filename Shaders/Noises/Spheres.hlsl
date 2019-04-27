#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"

/*[vk::binding(0, 0)]
ConstantBuffer< LWGC_PerFrame > frame;

RWTexture3D< float4 > noiseVolume;

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	float3 uv = i.dispatchThreadId.xyz / float3(32.0) - 1.0;

    // Basic domain repetition
    uv = frac(uv);

	noiseVolume[i.dispatchThreadId.xy] = half4(noise(uv * 10 + frame.time.x), 0, 0, 1);
}*/

#version 440
layout (binding = 0, rgba32f) uniform image3D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

void main() {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	imageStore(destTex, storePos, vec4(0.0,0.0,1.0,1.0));
}