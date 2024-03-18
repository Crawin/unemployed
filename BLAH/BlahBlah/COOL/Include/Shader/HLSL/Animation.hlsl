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
	float anim1PlayTime;
	float anim2Playtime;
	float animBlend;

	int anim1Frame;
	int anim2Frame;
};

#define MAX_BONE_LEN 64
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

	float interpolWegith = ceil(anim1PlayTime * ANIMATION_FPS) - anim1PlayTime * ANIMATION_FPS;

	matrix inverseMatrix = inverse(localMatrix);
	int idx, boneIdx;
	float weight;

	int currentFrame = floor(anim1PlayTime* ANIMATION_FPS);

	// for first anim
	for (int i = 0; i < 4; ++i)
	{
		// animation interpolation
		boneIdx = input.boneIndexs[i];
		weight = input.boneWeights[i];
		idx = boneIdx * anim1Frame + currentFrame;
		
		boneToWorld = mul(Bone[boneIdx], Animation[idx]);//lerp(Animation[idx + 1], Animation[idx], interpolWegith));
		//boneToWorld = Bone[boneIdx];	
		//boneToWorld = Animation[20 * floor(anim1PlayTime * 24.0f)];

		output.position += weight * mul(mul(mul(float4(input.position, 1.0f), localMatrix), boneToWorld), inverseMatrix).xyz;
		output.normal += weight * mul(mul(mul(input.normal, (float3x3) localMatrix),(float3x3)boneToWorld), (float3x3)inverseMatrix);
		output.tangent += weight * mul(mul(mul(input.tangent, (float3x3) localMatrix),(float3x3)boneToWorld), (float3x3)inverseMatrix);
	}

	output.position.y += anim1PlayTime * 10.0f;
	//output.position = mul(mul(float4(input.position, 1.0f), localMatrix), inverseMatrix).xyz;
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