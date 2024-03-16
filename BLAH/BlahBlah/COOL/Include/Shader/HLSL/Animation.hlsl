#include "ShaderBaseParameters.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
	uint4 boneIndexs : BONEINDEX;
	float4 boneWeights : BONEWEIGHT;
};



struct VS_OUTPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
};

cbuffer MaterialAnim : register(b0)
{
	uint boneIdx;
	uint anim1Idx;
	uint anim2Idx;
	float animBlend;

	float anim1PlayTime;
	float anim2Playtime;
};

#define MAX_BONE_LEN 64


struct BONEDATA
{
	matrix boneMat[MAX_BONE_LEN];
};

struct AnimData
{
	matrix anim[MAX_BONE_LEN];
};

StructuredBuffer<BONEDATA> BoneDataList : register(t0, space6);
StructuredBuffer<AnimData> AnimDataList : register(t0, space7);

VS_OUTPUT vs(VS_INPUT input)
{
	// do animation here
	BONEDATA bone = BoneDataList[boneIdx].boneMat;
	
	

	VS_OUTPUT output;
	output.position = input.position;
	output.normal = input.normal;
	output.tangent = input.tangent;
	output.uv = input.uv;

	return output;
}


[maxvertexcount(3)]
void gs(triangle VS_OUTPUT input[3], inout TriangleStream<VS_OUTPUT> outStream)
{
	// send to outStream
	outStream.Append(input[0]);
	outStream.Append(input[1]);
	outStream.Append(input[2]);
	outStream.RestartStrip();
}