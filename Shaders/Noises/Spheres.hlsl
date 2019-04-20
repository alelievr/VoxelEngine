#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"

[vk::binding(0, 0)]
ConstantBuffer< LWGC_PerFrame > frame;

RWTexture3D< float > noiseVolume;

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	float3 uv = i.dispatchThreadId.xyz / float3(32.0) - 1.0;

    // Basic domain repetition
    uv = frac(uv);

	noiseVolume[i.dispatchThreadId.xy] = half4(noise(uv * 10 + frame.time.x), 0, 0, 1);
}