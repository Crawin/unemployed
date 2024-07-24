
#include "ShaderBaseParameters.hlsl"


struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
};


struct ExtraData
{
	float data0;
	float data1;
	float data2;
	float data3;
};

cbuffer Material : register(b0)
{
	uint g_MatIdx0;
	uint g_MatIdx1;
	uint g_MatIdx2;
	uint g_MatIdx3;
	uint g_MatIdx4;
	float g_Extra0;
	float g_Extra1;
	float g_Extra2;
	ExtraData g_ExtraData[2];
};


struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
};

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;
	
	o.positionW = (float3) mul(float4(i.position, 1.0f), localMatrix);
	o.normalW = normalize(mul(i.normal, (float3x3) localMatrix));
	o.tangentW = normalize(mul(i.tangent, (float3x3) localMatrix));
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
	
	output.Albedo = float4(Tex2DList[g_MatIdx0].Sample(samplerWarp, i.uv));
	output.Roughness = float4(Tex2DList[g_MatIdx1].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Metalic = float4(Tex2DList[g_MatIdx2].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Ao =float4(Tex2DList[g_MatIdx3].Sample(samplerWarp, i.uv).rrr, 1.0f);

	float3 biTangent = cross(i.normalW, i.tangentW);
	float3x3 TBN = float3x3(i.tangentW, biTangent, i.normalW);
	float3 sampledNormal = normalize(Tex2DList[g_MatIdx4].Sample(samplerWarp, i.uv).rgb * 2.0f - 1.0f);

	float3 res = mul(sampledNormal, TBN);
	if (g_MatIdx4 <= 0) res = i.normalW;
	//output.Albedo =  float4(Tex2DList[g_MatIdx4].Sample(samplerWarp, i.uv));
	//output.NormalW = float4(normalize(mul(Tex2DList[g_MatIdx4].Sample(samplerWarp, i.uv).rgb, TBN)), g_ExtraData[0].data0);
	//output.NormalW = float4(i.normalW, 0.0f);
	output.NormalW = float4(res, g_Extra0);
	output.PositionW = float4(i.positionW.xyz, i.position.z);
	
	return output;
}
