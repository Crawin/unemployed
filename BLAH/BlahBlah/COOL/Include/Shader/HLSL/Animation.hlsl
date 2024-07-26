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

// current animation + extra datas
cbuffer MaterialAnim : register(b0)
{
	// anim data
	float4 anim1_BlendingWeights;
	int4 anim1_Frames;
	int4 anim1_Indices;

	float anim1_PlayTime;
	int anim1_AnimationMode;

	float animationBlendWeight;
	int boneDataIndex;
};

// before animation
cbuffer MaterialAnimExtra : register(b2)
{
	float4 anim2_BlendingWeights;
	int4 anim2_Frames;
	int4 anim2_Indices;

	float anim2_PlayTime;
	int anim2_AnimationMode;

	int animation1_AffactUpper;
	int animation1_AffactUpperIndex;
};

// animation mode: single anim, blend1dSpace, blend2dspace

#define ANIMATION_FPS 24.0f

//StructuredBuffer<matrix> Bone : register(t1, space8);
//StructuredBuffer<matrix> Animation_cur : register(t2, space9);
//StructuredBuffer<matrix> Animation_bef : register(t3, space10);

matrix CalculateAnimation(int bondIndex, int animDataIndex, float playTime, int animationFrame)
{
	StructuredBuffer<matrix> Bone = BonAnimDataList[boneDataIndex];
	StructuredBuffer<matrix> animation = BonAnimDataList[animDataIndex];

	int frameFloor = floor(playTime * ANIMATION_FPS) % animationFrame;
	int frameCeil = (frameFloor + 1) % animationFrame;

	float interpolWeight = ceil(playTime * ANIMATION_FPS) - playTime * ANIMATION_FPS;

	int idx = bondIndex * animationFrame + frameFloor;
	int idxNext = bondIndex * animationFrame + frameCeil;

	return mul(Bone[bondIndex], lerp(animation[idxNext], animation[idx], interpolWeight));
}

VS_OUTPUT vs(VS_INPUT input)
{
	// do animation here
	VS_OUTPUT output;

	output.position = float3(0.0f, 0.0f, 0.0f);
	output.normal = float3(0.0f, 0.0f, 0.0f);
	output.tangent = float3(0.0f, 0.0f, 0.0f);
	output.uv = input.uv;
	
	StructuredBuffer<matrix> Bone = BonAnimDataList[boneDataIndex];

	matrix boneToWorld = 0;

	// for first anim
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		// animation interpolation
		int boneIdx = input.boneIndexs[i];
		matrix anim1 = 0;

		switch(anim1_AnimationMode)
		{
		case 2:
			anim1 += (float)anim1_BlendingWeights.x * CalculateAnimation(boneIdx, anim1_Indices.x, anim1_PlayTime, anim1_Frames.x);
			anim1 += (float)anim1_BlendingWeights.y * CalculateAnimation(boneIdx, anim1_Indices.y, anim1_PlayTime, anim1_Frames.y);
			anim1 += (float)anim1_BlendingWeights.z * CalculateAnimation(boneIdx, anim1_Indices.z, anim1_PlayTime, anim1_Frames.z);
			anim1 += (float)anim1_BlendingWeights.w * CalculateAnimation(boneIdx, anim1_Indices.w, anim1_PlayTime, anim1_Frames.w);
			break;
		case 1:
			anim1 += (float)anim1_BlendingWeights.x * CalculateAnimation(boneIdx, anim1_Indices.x, anim1_PlayTime, anim1_Frames.x);
			anim1 += (float)anim1_BlendingWeights.y * CalculateAnimation(boneIdx, anim1_Indices.y, anim1_PlayTime, anim1_Frames.y);
			break;
		case 0:
			anim1 += (float)anim1_BlendingWeights.x * CalculateAnimation(boneIdx, anim1_Indices.x, anim1_PlayTime, anim1_Frames.x);
			break;
		}

		if (animationBlendWeight > 0 || animation1_AffactUpper >= 0) {
			matrix anim2 = 0;
			switch(anim2_AnimationMode)
			{
			case 2:
				anim2 += (float)anim2_BlendingWeights.x * CalculateAnimation(boneIdx, anim2_Indices.x, anim2_PlayTime, anim2_Frames.x);
				anim2 += (float)anim2_BlendingWeights.y * CalculateAnimation(boneIdx, anim2_Indices.y, anim2_PlayTime, anim2_Frames.y);
				anim2 += (float)anim2_BlendingWeights.z * CalculateAnimation(boneIdx, anim2_Indices.z, anim2_PlayTime, anim2_Frames.z);
				anim2 += (float)anim2_BlendingWeights.w * CalculateAnimation(boneIdx, anim2_Indices.w, anim2_PlayTime, anim2_Frames.w);
				break;
			case 1:
				anim2 += (float)anim2_BlendingWeights.x * CalculateAnimation(boneIdx, anim2_Indices.x, anim2_PlayTime, anim2_Frames.x);
				anim2 += (float)anim2_BlendingWeights.y * CalculateAnimation(boneIdx, anim2_Indices.y, anim2_PlayTime, anim2_Frames.y);
				break;
			case 0:
				anim2 += (float)anim2_BlendingWeights.x * CalculateAnimation(boneIdx, anim2_Indices.x, anim2_PlayTime, anim2_Frames.x);
				break;
			}

			if (animation1_AffactUpper > 0) {
				//boneToWorld += input.boneWeights[i] * anim1;
				//if (2 <= boneIdx && boneIdx <= animation1_AffactUpperIndex)
				if (boneIdx <= animation1_AffactUpperIndex)
					boneToWorld += input.boneWeights[i] * anim1;
				else
					boneToWorld += input.boneWeights[i] * anim2;
			}
			else
				boneToWorld += input.boneWeights[i] * lerp(anim1, anim2, animationBlendWeight);
		}
		else
			boneToWorld += input.boneWeights[i] * anim1;	
	}

	output.position = mul(float4(input.position, 1.0f), boneToWorld).xyz;
	output.normal = mul(input.normal,(float3x3)boneToWorld);
	output.tangent = mul(input.tangent,(float3x3)boneToWorld);


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