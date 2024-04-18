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
	float anim2PlayTime;
	float animBlend;

	int anim1Frame;
	int anim2Frame;

	int anim1Idx;
	int anim2Idx;
	int boneDataIdx;
};

#define ANIMATION_FPS 24.0f

//StructuredBuffer<matrix> Bone : register(t1, space8);
//StructuredBuffer<matrix> Animation_cur : register(t2, space9);
//StructuredBuffer<matrix> Animation_bef : register(t3, space10);

VS_OUTPUT vs(VS_INPUT input)
{
	// do animation here
	VS_OUTPUT output;

	output.position = float3(0.0f, 0.0f, 0.0f);
	output.normal = float3(0.0f, 0.0f, 0.0f);
	output.tangent = float3(0.0f, 0.0f, 0.0f);
	output.uv = input.uv;
	
	StructuredBuffer<matrix> Bone = BonAnimDataList[boneDataIdx];
	StructuredBuffer<matrix> Animation_cur = BonAnimDataList[anim1Idx];
	StructuredBuffer<matrix> Animation_bef = BonAnimDataList[anim2Idx];

	matrix boneToWorld = 0;
	matrix animByFrame;

	float anim1InterpolWegith = ceil(anim1PlayTime * ANIMATION_FPS) - anim1PlayTime * ANIMATION_FPS;
	float anim2InterpolWegith = ceil(anim2PlayTime * ANIMATION_FPS) - anim2PlayTime * ANIMATION_FPS;

	matrix inverseMatrix = inverse(localMatrix);
	float weight;

	int currentFrame_cur = floor(anim1PlayTime * ANIMATION_FPS);
	int currentFrame_bef = floor(anim2PlayTime * ANIMATION_FPS);

	// for first anim
	for (int i = 0; i < 4; ++i)
	{
		// animation interpolation
		float weight = input.boneWeights[i];
		int boneIdx = input.boneIndexs[i];
		int idx1 = boneIdx * anim1Frame + currentFrame_cur;
		int idx2 = boneIdx * anim2Frame + currentFrame_bef;
		
		matrix anim1 = mul(Bone[boneIdx], lerp(Animation_cur[idx1 + 1], Animation_cur[idx1], anim1InterpolWegith));

		if (animBlend > 0){
			matrix anim2 = mul(Bone[boneIdx], lerp(Animation_bef[idx2 + 1], Animation_bef[idx2], anim2InterpolWegith));
			boneToWorld += weight * lerp(anim1, anim2, animBlend);
		}
		else
			boneToWorld += weight * anim1;	
		//boneToWorld = Bone[boneIdx];	
		//boneToWorld = Animation[20 * floor(anim1PlayTime * 24.0f)];
	}

	output.position = mul(mul(mul(float4(input.position, 1.0f), localMatrix), boneToWorld), inverseMatrix).xyz;
	output.normal = mul(mul(mul(input.normal, (float3x3) localMatrix),(float3x3)boneToWorld), (float3x3)inverseMatrix);
	output.tangent = mul(mul(mul(input.tangent, (float3x3) localMatrix),(float3x3)boneToWorld), (float3x3)inverseMatrix);


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