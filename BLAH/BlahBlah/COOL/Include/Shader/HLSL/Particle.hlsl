
#include "ShaderBaseParameters.hlsl"

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

cbuffer WorldMatrix : register(b2)
{
	float2 g_Size;
};

struct VS_INPUT
{
	float3 position : POSITION;
};

struct VS_OUTPUT
{
	float3 position : POSITION;
};



VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT o;

	o.position = i.position;

	return o;
}

struct GS_OUTPUT
{
	float4 position : SV_Position;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

// billboard
[maxvertexcount(4)]
void gs(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> outStream)
{
	float x = g_Size.x / 2.0f;
	float y = g_Size.y / 2.0f;

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = normalize(cameraPosition - input[0].position);
	float3 right = cross(up, look);

	float4 vertices[4] = { float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f) };
	vertices[0] = float4(input[0].position + (-x * right - y * up), 1.0f);
	vertices[1] = float4(input[0].position + (-x * right + y * up), 1.0f);
	vertices[2] = float4(input[0].position + (+x * right + y * up), 1.0f);
	vertices[3] = float4(input[0].position + (+x * right - y * up), 1.0f);

	float2 uvs[4] = { 
		float2(0.0f, 1.0f), 
		float2(0.0f, 0.0f), 
		float2(1.0f, 1.0f), 
		float2(1.0f, 0.0f) 
	};

	GS_OUTPUT output;
	matrix VP = mul(viewMatrix, projectionMatrix);

	for (int i = 0; i < 4; ++i) {
		output.position = mul(vertices[i], VP);
		output.positionW = vertices[i].xyz;
		output.normalW = look;
		output.uv = uvs[i];
		outStream.Append(output);
	}
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

PS_MRT_OUTPUT ps(GS_OUTPUT i)
{
	// fbx파일을 유니티에서 뽑아와서 해둔 임시 코드
	i.uv.y = 1 - i.uv.y;
	
	PS_MRT_OUTPUT output;
	
	output.Albedo = float4(Tex2DList[g_MatIdx0].Sample(samplerWarp, i.uv));
	output.Roughness = float4(Tex2DList[g_MatIdx1].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Metalic = float4(Tex2DList[g_MatIdx2].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.Ao =float4(Tex2DList[g_MatIdx3].Sample(samplerWarp, i.uv).rrr, 1.0f);
	output.NormalW = float4(i.normalW, g_Extra0);
	output.PositionW = float4(i.positionW.xyz, i.position.z);
	
	return output;
}
