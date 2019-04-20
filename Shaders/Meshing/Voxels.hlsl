#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"

[vk::binding(0, 0)]
ConstantBuffer< LWGC_PerFrame > frame;

[vk::binding(0, 1)]
uniform Texture3D< half > noiseVolume;

[numthreads(8, 8, 1)]
void        main(ComputeInput i)
{
}