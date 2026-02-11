#include "CommonVS.hlsli"
#include "CommonMath.hlsli"

VS_OUTPUT_POS_UV main(VS_INPUT_POS_UV input, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT_POS_UV output;
    
    float3 centerWorldPos = float3(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43);
    
    float3 cameraRight = float3(ViewMatrix._11, ViewMatrix._12, ViewMatrix._13);
    float3 flatRight = normalize(float3(cameraRight.x, 0.0f, cameraRight.z));
    
    float3 positionOffset = (flatRight * input.Position.x + float3(0.0f, 1.0f, 0.0f) * input.Position.y) * ImageScale;
    float4 worldPos = float4((centerWorldPos + positionOffset), 1.0f);
    
    float rndTime = Rand(LowBias32(instanceID));
    float rndPosSeed = Rand(LowBias32(instanceID + 1u));
    
    float randomTime = fmod(EclipsedTime + rndTime, 1.0f);
    float3 pos = float3(Rand2(rndPosSeed), randomTime);
    
    worldPos.xyz += mul(pos, (float3x3) WorldMatrix);
    
    output.Position = mul(worldPos, VPMatrix);
    output.UV = (input.UV * UVScale) + UVOffset;

    return output;
}