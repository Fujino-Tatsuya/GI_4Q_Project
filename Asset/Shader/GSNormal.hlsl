#include "CommonVS.hlsli"
#include "CommonPS.hlsli"

cbuffer NormalViewProjection : register(b0)
{
    matrix NormalVPMatrix; // 노멀을 그릴 때 사용할 뷰 행렬과 투영 행렬의 곱
};

struct GS_OUTPUT_POS_COLOR
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

[maxvertexcount(2)]
void main(point VS_OUTPUT_STD input[1], inout LineStream<GS_OUTPUT_POS_COLOR> outputStream)
{
    GS_OUTPUT_POS_COLOR output;
    
    // Start point
    output.Position = mul(input[0].WorldPosition, NormalVPMatrix);
    output.Color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    outputStream.Append(output);
    
    // End point
    float4 normalEndPos = input[0].WorldPosition + float4(normalize(input[0].TBN[2]), 0.0f);
    output.Position = mul(normalEndPos, NormalVPMatrix);
    output.Color = float4(0.0f, 1.0f, 1.0f, 1.0f);
    outputStream.Append(output);
    
    outputStream.RestartStrip();
}