
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normalOnPos : NORMAL;
};

struct PS_MRT_OUTPUT
{
	float4 Albedo : SV_TARGET0;
	float4 Roughness : SV_TARGET1;
	float4 Metalic : SV_TARGET2;
	float4 Specular : SV_TARGET3;
	float4 NormalW : SV_TARGET4;
	float4 PositionW : SV_Target5;
};

cbuffer MaterialSky : register(b0)
{
	float4 DayLight;
	float4 MoonLight;
	float4 SunSetLight;
	
	int LightResourceIdx;
	int MainLightIdx;
	float LightAngle;
};

VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); }

	output.normalOnPos = mul(mul(normalize(output.position), inverse(projectionMatrix)).xyz, (float3x3)inverse(viewMatrix));

	return output; 
}



PS_MRT_OUTPUT ps(VS_OUTPUT i)
{
	PS_MRT_OUTPUT output;

	StructuredBuffer<LIGHT> lights = LightDataList[LightResourceIdx];

	//float4 frustumPos = mul(i.position, inverseMatrix(projectionMatrix));
	//float3 viewDir = mul(frustumPos.xyz / frustumPos.w, (float3x3)viewMatrix)

	float4 mainLight, subLight;
	// day light is main
	float angle = LightAngle;
	if (angle < 180.0f) {
		mainLight = DayLight;
		subLight = MoonLight;
	}
	else {
		mainLight = MoonLight;
		subLight = DayLight;
		angle -= 180.0f;
	}

	float weight = abs((sin(radians(angle))));

	float4 currentDayLight = lerp(SunSetLight, mainLight, weight);
	float4 nextDayLight = lerp(SunSetLight, subLight, weight);

	float4 color = lerp(currentDayLight, nextDayLight, i.normalOnPos.y);

	

	//float4 currentDayLight;

	//color.xyz = i.normalOnPos;
	output.Albedo = currentDayLight;
	//output.Albedo = lights[MainLightIdx].m_LightColor * color;
	output.Roughness = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.Metalic = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.NormalW = float4(normalize(i.normalOnPos), 0.0f);//float4(i.normalOnPos.y, i.normalOnPos.y, i.normalOnPos.y, 0.0f);
	output.PositionW = float4(0.0f, 0.0f, 0.0f, 0.0f);

	return output;
}
