#include "CommonVS.hlsli"
#include "CommonMath.hlsli"

VS_OUTPUT_POS_WORLD_UV main(VS_INPUT_POS_UV input, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT_POS_WORLD_UV output;
    
    float4 worldPos = mul(input.Position, WorldMatrix);
    
    float rndTime = Rand(LowBias32(instanceID));
    float rndPosSeed = Rand(LowBias32(instanceID + 1u));
    
    float randomTime = fmod(EclipsedTime + rndTime, 1.0f);
    float3 pos = float3(Rand2(rndPosSeed), randomTime);
    
    worldPos.xyz += mul(pos, (float3x3) WorldMatrix);
    
    output.Position = mul(worldPos, VPMatrix);
    output.WorldPosition = worldPos;
    output.UV = (input.UV * UVScale) + UVOffset;

    return output;
}