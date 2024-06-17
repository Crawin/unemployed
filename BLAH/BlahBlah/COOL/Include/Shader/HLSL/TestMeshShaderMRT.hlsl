
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
	
	output.Albedo = float4(Tex2DList[matIdx0.x].Sample(samplerWarp, i.uv));
	output.Roughness = float4(Tex2DList[matIdx0.y].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Metalic = float4(Tex2DList[matIdx0.z].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Ao =float4(Tex2DList[matIdx0.w].Sample(samplerWarp, i.uv).rrr, 1.0f);

	float3 biTangent = cross(i.normalW, i.tangentW);
	float3x3 TBN = float3x3(i.tangentW, biTangent, i.normalW);
	float3 sampledNormal = normalize(Tex2DList[matIdx1.x].Sample(samplerWarp, i.uv).rgb * 2.0f - 1.0f);

	float3 res = mul(sampledNormal, TBN);
	if (matIdx1.x <= 0) res = i.normalW;
	//output.Albedo =  float4(Tex2DList[matIdx1.x].Sample(samplerWarp, i.uv));
	output.NormalW = float4(normalize(mul(Tex2DList[matIdx1.x].Sample(samplerWarp, i.uv).rgb, TBN)), 0.0f);
	//output.NormalW = float4(i.normalW, 0.0f);
	output.NormalW = float4(res, 0.0f);
	output.PositionW = float4(i.positionW.xyz, i.position.z);
	
	return output;
}
