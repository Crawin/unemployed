
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer Material : register(b0)
{
	uint m_BaseNoiseTexture;
	uint m_NoiseTextureMoveMount;
	uint2 texPadding;

	uint texPad2;
	float m_ElapsedTime;
	//float2 m_Padding;

	//uint4 matIdx2;
	//uint4 matIdx3;
};

VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.5f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.5f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.5f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.5f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.5f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.5f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return output; 
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	float2 finUV = input.uv * 3.0f;
	finUV.x += sin(m_ElapsedTime * 10.0f);
	finUV.y -= m_ElapsedTime;

	float4 noiseColor = Tex2DList[m_BaseNoiseTexture].Sample(samplerWarp, finUV);
	
	return float4(noiseColor.rgb, 0.2f);

}
