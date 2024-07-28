
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};


cbuffer Material : register(b0)
{
	uint m_BaseNoiseTexture;
	uint3 texPadding;

	uint texPad2;
	float m_ElapsedTime;
	//float2 m_Padding;

	//uint4 matIdx2;
	//uint4 matIdx3;
};


VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.5f, 0.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.5f, 0.0f); output.uv = float2(1.0f, 0.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.5f, 0.0f); output.uv = float2(1.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.5f, 0.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.5f, 0.0f); output.uv = float2(1.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.5f, 0.0f); output.uv = float2(0.0f, 1.0f); }

	return output; 
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	// tile mount 20.0f
	float2 uvs = input.uv * 20.0f + sin(m_ElapsedTime);
	float4 result = Tex2DList[m_BaseNoiseTexture].Sample(samplerWarp, uvs);
	return float4(result.rgb, 0.2f);
}
