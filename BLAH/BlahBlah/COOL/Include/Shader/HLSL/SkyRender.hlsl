
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
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

struct PS_MRT_OUTPUT
{
	float4 Albedo : SV_TARGET0;
	float4 Roughness : SV_TARGET1;
	float4 Metalic : SV_TARGET2;
	float4 AO : SV_TARGET3;
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

VS_OUTPUT vs(VS_INPUT i)
{
	VS_OUTPUT output;

	output.position = mul(float4(mul(i.normal, (float3x3)viewMatrix), 1.0f), projectionMatrix);
	output.normal = -i.normal;

	return output; 
}

#define ATMOSPHERE_HEIGHT 32.0f
#define HORIZONTAL_LENGTH 640.0f

float3 CalculateRayleighScattering(float3 lightDirection, float3 viewDirection, float3 normal)
{
	// Constants for Rayleigh scattering
	const float3 waveLengths  = float3(0.650f, 0.570f, 0.475f);
	const float3 k =  0.0025 / (waveLengths * waveLengths * waveLengths * waveLengths);

	float cosTheta = (dot(normalize(viewDirection), normalize(normal)) + 1) / 2.0f;
	float3 rayleigh = k * (1.0 + cosTheta * cosTheta);

	float cosPhi = (dot(normalize(lightDirection), normalize(normal)) + 1) / 2.0f;
	float sunlightIntensity = max(0.0, cosPhi) * 20.0;

	float3 skyColor = rayleigh * sunlightIntensity;
	skyColor = pow(skyColor, (1.0 / 2.2));

	return skyColor;
}

float3 CalculateLightSource(float3 normal, float3 lightDirection)
{
	float dotRes = dot(normal, lightDirection);
	float mult = pow(abs(dotRes), 128.0f);

	return float3(1, 1, 1) * mult;
}

PS_MRT_OUTPUT ps(VS_OUTPUT i)
{
	PS_MRT_OUTPUT output;

	StructuredBuffer<LIGHT> lights = LightDataList[LightResourceIdx];

	LIGHT mainLight = lights[MainLightIdx];

	output.Roughness = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.Metalic = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.AO = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.NormalW = float4(normalize(i.normal), 0.0f);

	float4 pos = float4(cameraPosition, 0.0f) + 5000.0f * -output.NormalW;
	output.PositionW = pos;

	float3 viewVector = normalize(pos.xyz - cameraPosition);

	float angle = dot(mainLight.m_Direction, i.normal);
	//clip(angle);

	float3 albedo = CalculateRayleighScattering(-normalize(mainLight.m_Direction), viewVector, -i.normal) + CalculateLightSource(i.normal, mainLight.m_Direction);
	albedo *= mainLight.m_LightColor;
	//color.xyz = i.normal;
	output.Albedo = float4(albedo, 0.0f);
	//output.Albedo = float4(lights[MainLightIdx].m_LightAmbient.rgb, 0.0f);
	//output.Albedo = float4(1,0,1, 0.0f);
	//output.Albedo = float4(1,1,1,0);
	//output.Albedo = float4(density,density,density,1.0f);
	//output.Albedo = lights[MainLightIdx].m_LightColor * color;
	return output;
}
