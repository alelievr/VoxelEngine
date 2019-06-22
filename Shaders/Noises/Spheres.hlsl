#include "Common/InputCompute.hlsl"
#include "Common/UniformStructs.hlsl"

[vk::binding(0, 0)]
ConstantBuffer< LWGC_PerFrame > frame;

[vk::binding(0, 1)]
RWTexture3D< half > noiseVolume;

float2 hash(float2 x)
{
    const float2 k = float2(0.3183099, 0.3678794);
    x = x * k + k.yx;
    return -1.0 + 2.0 * frac(16.0 * k * frac(x.x * x.y * (x.x + x.y)));
}

float simpleNoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);

	float2 u = f*f*(3.0-2.0*f);

    return lerp(lerp(dot(hash(i + float2(0.0,0.0)), f - float2(0.0,0.0)),
                     dot(hash(i + float2(1.0,0.0)), f - float2(1.0,0.0)), u.x),
                lerp(dot(hash(i + float2(0.0,1.0)), f - float2(0.0,1.0)),
                     dot(hash(i + float2(1.0,1.0)), f - float2(1.0,1.0)), u.x), u.y);
}

[numthreads(8, 8, 8)]
void        main(ComputeInput i)
{
	float3 uv = i.dispatchThreadId.xyz / float3(128.0);

    // Basic domain repetition
    uv = frac(uv * 1) * 2.0 - 1.0; /// mad

    float density = length(uv) - 1.3;

    // density must be within 0 and 1 range
	noiseVolume[i.dispatchThreadId.xyz] = saturate(density);

    // uint3 m = (i.dispatchThreadId.xyz / 1) % 2;
	// noiseVolume[i.dispatchThreadId.xyz] = m.x ^ m.y ^ m.z;
}
