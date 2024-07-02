
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return output; 
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	return float4(Tex2DList[matIdx0.x].Sample(samplerWarp, input.uv));;
}
