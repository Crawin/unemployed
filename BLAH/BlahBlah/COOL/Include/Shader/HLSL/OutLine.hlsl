
#include "ShaderBaseParameters.hlsl"


struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
};



struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
};

#define VERTEX_OFFSET 5.0f
#define MULT_MOUNT 1.1f

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;

	float3x3 mulOnePOne = float3x3(
		MULT_MOUNT, 0.0, 0.0,
		0.0, MULT_MOUNT, 0.0,
		0.0, 0.0, MULT_MOUNT
		);

	
	float3 pos = mul(i.position, mulOnePOne);

	o.positionW = (float3) mul(float4(pos, 1.0f), localMatrix);
	o.normalW = normalize(mul(i.normal, (float3x3) localMatrix));
	o.tangentW = normalize(mul(i.tangent, (float3x3) localMatrix));

	//o.positionW += o.normalW * VERTEX_OFFSET;

	o.position = mul(mul(float4(o.positionW, 1.0f), viewMatrix), projectionMatrix);
	
	o.uv = i.uv;
	
	return o;
}

struct PS_MRT_OUTPUT
{
	float4 Albedo : SV_TARGET0;
	float4 Roughness : SV_TARGET1;
	float4 Metalic : SV_TARGET2;
	float4 Ao : SV_TARGET3;
	float4 NormalW : SV_TARGET4;
	float4 PositionW : SV_Target5;
};

PS_MRT_OUTPUT ps(VS_OUTPUT i)
{
	// fbx파일을 유니티에서 뽑아와서 해둔 임시 코드
	i.uv.y = 1 - i.uv.y;
	
	PS_MRT_OUTPUT output;
	
	output.Albedo = float4(1.0f, 1.0f, 0.0f, 1.0f);
	output.Roughness = float4(0.0f, 0.0f, 0.0f, 1.0f);
	output.Metalic = float4(1.0f, 1.0f, 1.0f, 1.0f);
	output.Ao =float4(1, 1, 1, 1.0f);
	output.NormalW = float4(i.normalW, 0.0f);
	output.PositionW = float4(i.positionW.xyz, i.position.z);
	
	return output;
}
