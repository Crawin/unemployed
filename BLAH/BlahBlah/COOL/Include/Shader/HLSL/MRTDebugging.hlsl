
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

	clip(0.5f - input.uv.x);
	clip(0.5f - input.uv.y);

	input.uv.x *= 6;
	input.uv.y *= 4;
	
	int idx = floor(input.uv.x) + (3 * floor(input.uv.y));

	// sample all
	//float4 albedoColor = float4(Tex2DList[ALBEDO].Sample(samplerWarp, input.uv));
	//float4 roughness = float4(Tex2DList[ROUGHNESS].Sample(samplerWarp, input.uv));
	//float4 metalic = float4(Tex2DList[METALIC].Sample(samplerWarp, input.uv));
	//float4 specular = float4(Tex2DList[SPECULAR].Sample(samplerWarp, input.uv));
	//float4 normalW = float4(Tex2DList[NORMALW].Sample(samplerWarp, input.uv));
	//float4 positionW = float4(Tex2DList[materialIndex[POSITIONW]].Sample(samplerWarp, input.uv));
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);

	return float4(Tex2DList[idx].Sample(samplerWarp, input.uv));;
}
