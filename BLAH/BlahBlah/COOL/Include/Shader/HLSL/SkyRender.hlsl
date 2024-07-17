
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

float CalculateLayDistance(float3 normal, float3 lay)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);

	// atmosphere height = 32km
	// horizontal length = 640km
	float length = lerp(ATMOSPHERE_HEIGHT, HORIZONTAL_LENGTH, dot(up, lay));

	//float density = length * (1 + dot(normal, lay));

	return length;
}

float CalculateColorMult(float wavelength, float dist, float3 normal, float3 lay)
{
	const float n = 1.0003; // 대기의 굴절률
	const float N = 2.545e25; // 분자수 밀도
	const float pn = 0.035; // 편광 계수

	float waveLengthM = wavelength * 1e-9;

	float rayleighCoefficient = (8.0 * pow(PI, 3.0) * pow((n * n - 1.0), 2.0)) / (3.0 * N * pow(waveLengthM, 4.0)) * (6.0 + 3.0 * pn) / (6.0 - 7.0 * pn);

	float attenuation = exp(-rayleighCoefficient * dist);

	return float3(attenuation, attenuation, attenuation);
}

float3 CalculateRayleighScattering(float3 lightDirection, float3 viewDirection, float3 normal)
{
	// Constants for Rayleigh scattering
	float3 waveLength = float3(0.650f, 0.570f, 0.475f); // Wavelengths of red, green, blue light
	float3 invWaveLength4 = float3(1.0f / pow(waveLength.x, 4), 1.0f / pow(waveLength.y, 4), 1.0f / pow(waveLength.z, 4));
	float rayleighCoefficient = 0.0025f; // Scattering coefficient
	float rayleighIntensity = 0.3f; // Intensity of the Rayleigh scattering

	float3 lightVec = normalize(lightDirection);
	float3 viewVec = normalize(viewDirection);
	float3 halfVec = normalize(lightVec + viewVec);
	float NdotL = max(dot(normal, lightVec), 0.0f);

	// Rayleigh phase function
	float g = 0.76f; // The anisotropy factor for Rayleigh scattering
	float phase = (3.0f / (16.0f * PI)) * (1.0f + pow(dot(lightVec, viewVec), 2.0f));
	
	float3 scattering = invWaveLength4 * rayleighCoefficient * phase * rayleighIntensity;

	return scattering * NdotL;
}


PS_MRT_OUTPUT ps(VS_OUTPUT i)
{
	PS_MRT_OUTPUT output;

	StructuredBuffer<LIGHT> lights = LightDataList[LightResourceIdx];

	//float4 frustumPos = mul(i.position, inverseMatrix(projectionMatrix));
	//float3 viewDir = mul(frustumPos.xyz / frustumPos.w, (float3x3)viewMatrix)

	//float4 mainLight, subLight;
	//// day light is main
	//float angle = LightAngle;
	//if (angle > 180.0f) {
	//	mainLight = DayLight;
	//	subLight = MoonLight;
	//	angle -= 180.0f;
	//}
	//else {
	//	mainLight = MoonLight;
	//	subLight = DayLight;
	//}

	//float weight = abs((sin(radians(angle))));

	//float4 currentDayLight = lerp(SunSetLight, mainLight, weight);
	//float4 nextDayLight = lerp(SunSetLight, subLight, weight);

	//float4 color = lerp(currentDayLight, nextDayLight, i.normal.y);


	//currentDayLight = lerp(SunSetLight, mainLight, dist / 630);

	//color.xyz = i.normal;
	output.Albedo = float4(CalculateRayleighScattering(lights[MainLightIdx].m_Direction, i.normal, i.normal), 1.0f);
	//output.Albedo = float4(density,density,density,1.0f);
	//output.Albedo = lights[MainLightIdx].m_LightColor * color;
	output.Roughness = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.Metalic = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.AO = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.NormalW = float4(normalize(i.normal), 0.0f);
	
	float4 pos = float4(cameraPosition, 0.0f) + 5000.0f * -output.NormalW;
	output.PositionW = pos;

	return output;
}
