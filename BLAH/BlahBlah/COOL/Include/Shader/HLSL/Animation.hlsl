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
	uint boneLenth;
	float animBlend;
	float anim1PlayTime;
	float anim2Playtime;
	
	int anim1Frame;
	int anim2Frame;
};

#define MAX_BONE_LEN 64
#define MAX_ANIMATION_FRAME 100
#define ANIMATION_FPS 24.0f

StructuredBuffer<matrix> Bone : register(t1, space8);
StructuredBuffer<matrix> Animation : register(t2, space9);

VS_OUTPUT vs(VS_INPUT input)
{
	// do animation here
	VS_OUTPUT output;

	output.position = float3(0.0f, 0.0f, 0.0f);
	output.normal = float3(0.0f, 0.0f, 0.0f);
	output.tangent = float3(0.0f, 0.0f, 0.0f);
	output.uv = input.uv;
	
	matrix boneToWorld;
	matrix animByFrame;

	float frameInterval = 1 / ANIMATION_FPS;
	float interpolWegith = ceil(anim1PlayTime * ANIMATION_FPS) - anim1PlayTime * ANIMATION_FPS;

	int idx1, idx2, boneIdx;
	float weight;
	// for first anim
	for (int i = 0; i < 4; ++i)
	{
		// animation interpolation
		boneIdx = input.boneIndexs[i];
		weight = input.boneWeights[i];
		idx1 = boneIdx * anim1Frame + floor(anim1PlayTime * ANIMATION_FPS);
		idx2 = boneIdx * anim1Frame + ceil(anim1PlayTime * ANIMATION_FPS);
		
		boneToWorld = mul(Bone[boneIdx], Animation[idx1]);//lerp(Animation[idx1], Animation[idx2], interpolWegith));
		//boneToWorld = Bone[boneIdx];

		output.position += weight * mul(float4(input.position, 1.0f), boneToWorld).xyz;
		output.normal += weight * mul(input.normal, (float3x3) boneToWorld);
		output.tangent += weight * mul(input.tangent, (float3x3) boneToWorld);
	}

	//output.position = input.position;
	//output.normal = input.normal;
	//output.tangent = input.tangent;


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