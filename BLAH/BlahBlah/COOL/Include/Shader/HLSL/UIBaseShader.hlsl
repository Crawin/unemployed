
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

cbuffer MaterialUI : register(b0)
{
	int textureIdx;
	int spriteSizeX;
	int spriteSizeY;
	int curSprite;

	int UIcenterX;
	int UIcenterY;
	int UIwidth;
	int UIheight;

	int screenWidth;
	int screenHeight;

};


VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0,0);}
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1,0);}
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1,1);}

	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0,0);}
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1,1);}
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0,1);}
	
	output.position.x *= 0.5f * UIwidth;
	output.position.y *= 0.5f * UIheight;

	output.position.x += UIcenterX - (float)(screenWidth) * 0.5f;
	output.position.y += (float)(screenHeight) * 0.5f - UIcenterY;

	output.position.x /= screenWidth;
	output.position.y /= screenHeight;
	output.position.x *= 2.0f;
	output.position.y *= 2.0f;

	return output;
}


float4 ps(VS_OUTPUT i) : SV_TARGET
{
	return float4(Tex2DList[textureIdx].Sample(samplerWarp, i.uv));
}
