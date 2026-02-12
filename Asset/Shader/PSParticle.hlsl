#include "CommonPS.hlsli"

PS_SCENE_OUTPUT main(PS_INPUT_POS_WORLD_UV input)
{
    PS_SCENE_OUTPUT output;
    
    float4 toCamera = input.WorldPosition - CameraPosition;
    float distanceSqToCamera = dot(toCamera, toCamera);
    
    output.Color = baseColorTexture.Sample(SamplerLinearWrap, input.UV) * ParticleBaseColor;
    output.Color.a *= saturate(distanceSqToCamera * 0.025f);
    output.ThresholdColor = output.Color * ParticleEmission; // 이거 factor으로 할지 그냥 더할지 고민중

    return output;
}