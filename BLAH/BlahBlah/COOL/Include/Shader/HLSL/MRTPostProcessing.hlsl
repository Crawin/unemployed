
#include "ShaderBaseParameters.hlsl"

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_OUTPUT vs(uint vtxID : SV_VertexID)
{
	VS_OUTPUT output;

	if (vtxID == 0)			{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 1)	{ output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (vtxID == 2)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	
	else if (vtxID == 3)	{ output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (vtxID == 4)	{ output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (vtxID == 5)	{ output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return output; 
}

cbuffer PostMaterial : register(b0)
{
	int g_AlbedoIndex;
	int g_RoughnessIndex;
	int g_MetallicIndex;
	int g_AOIndex;
	int g_NormalIndex;
	int g_PositionIndex;
	int g_LightSize;
	int g_LightDataIndex;
};

float ShadowCalculate(float4 worldPos, float dotNormal, int camIdx, int mapIdx)
{
	StructuredBuffer<CameraData> shadowCams = ShadowCameraDataList[camIdx];
	CameraData cam = shadowCams[0];
	float4 fragPos = mul(mul(worldPos, cam.viewMatrix), cam.projectionMatrix);
	//float3 ndc = fragPos.xyz / fragPos.w * 0.5f + 0.5f;
	float3 ndc = fragPos.xyz / fragPos.w;
	ndc.xy *= 0.5f;
	ndc.xy += 0.5f;
	//ndc.z *= 0.5f;
	//ndc.z += 0.5f;
	ndc.y = 1 - ndc.y;
	float shadow = 0.0;

	float bias = max(0.00005 * (1.0 - dotNormal), 0.000005);

	[unroll]
	for (int i = -1; i <= 1; ++i) {
		[unroll]
		for (int j = -1; j <= 1; ++j) {
			float depth = Tex2DList[mapIdx].Sample(samplerClamp, ndc.xy/*).r;// */+ (float2(i, j) / 8192.0f)).r;
			shadow += (depth + bias) < ndc.z ? 0.0 : 1.0;  
		}
	}

	return shadow / 9.0f;
}


float3 DiffuseBRDF(float3 albedo, float3 lightColor)
{
	// simple lambert
	return (albedo / PI) * lightColor;
}

// normal distribute function
float D_GGX(float NdotH, float roughness)
{
	// Trowbridge-Reitz GGX
	float r2 = roughness * roughness;
	float r4 = r2 * r2;
	float NdH2 = NdotH * NdotH;

	return r2 / (PI * pow((NdH2 * (r2 - 1.0f) + 1.0f), 2.0f));
}

// fresnel
float3 F_Schlick(float3 specColor, float theta)
{
	return specColor + (1.0f - specColor) * pow(1.0f - theta, 5.0f);
}

// geometric shadowing
float G_SchlickGGX(float NdotH, float roughness)
{
	float no = NdotH;
	float de = NdotH * (1 - roughness) + (pow(roughness + 1, 2.0f) * 0.125);

	return no / de;
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
	float v = G_SchlickGGX(NdotL, roughness);
	float l = G_SchlickGGX(NdotV, roughness);

	return v * l;
}

float3 SpecularBRDF(float3 albedo, float3 normal, float3 viewDir, float3 lightDir, float roughness, float metalic)
{
	// cook-torrance
	// divide 4 or PI
	// D: Normal Distribute Function
	// F: Fresnel
	// G: Geometric Shadowing
	// D*F*G / (4*dot(normal,light)*dot(normal,view))

	float3 halfDir = normalize(viewDir + lightDir);

	float NdotH = max(dot(normal, halfDir), 0.0f);
	float VdotH = max(dot(viewDir, halfDir), 0.0f);
	float NdotV = max(dot(normal, viewDir), 0.0f);
	float NdotL = max(dot(normal, lightDir), 0.0f);

	float3 specularColor = float3(0.04, 0.04, 0.04);
	specularColor = lerp(specularColor, albedo, metalic);

	float D = D_GGX(NdotH, roughness);
	float3 F = F_Schlick(specularColor, VdotH);
	float G = G_Smith(NdotV, NdotL, roughness);
	//float G = G_SmithSchlickGGX(NdotV, roughness);

	float3 result = (D * F * G) / max(PI * NdotV * NdotL, 0.001f);

	return result;
}

float3 PBRLighting(int idx, float3 normal, float3 viewDir, float3 lightDir, float4 albedo, float roughness, float metalic, float ao, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[g_LightDataIndex];

	float shadowFactor = 1;
	if (lights[idx].m_CameraIdx > 0) {
		float dotNormal = dot(normal, -lights[idx].m_Direction);
		shadowFactor = ShadowCalculate(
							float4(worldPosition, 1.0f), 
							dotNormal, 
							lights[idx].m_CameraIdx, 
							lights[idx].m_ShadowMapResults.x);
	}

	float3 diff = DiffuseBRDF(albedo.rgb, lights[idx].m_LightAmbient.rgb) ;
	float3 spec = 	SpecularBRDF(albedo.rgb, normal, viewDir, lightDir, roughness, metalic) 
					* lights[idx].m_LightColor.rgb
					* lights[idx].m_Intensity
					* shadowFactor;

	//if (shadowFactor == 1 && lights[idx].m_LightType == 0) return float3(1,0,0);

	return diff + spec;
}

float CalculateDistanceFactor(float3 lightPos, float3 worldPos, float maxDistance)
{
	float3 lightToPos = worldPos - lightPos;
	float distance = length(lightToPos);

	// distance factor
	return max(1 - distance / maxDistance, 0);
}

float CalculateAngleFactor(float3 lightPos, float3 worldPos, float3 lightDir, float maxAngle)
{
	float3 lightToPos = normalize(worldPos - lightPos);
	float actualAngle = acos(dot(lightToPos, lightDir));

	float falloutAngle = 2.0f * maxAngle / 3.0f;



	//if (actualAngle > maxAngle) return 0;
	//return 1;


	//return lerp(0, 1, actualAngle / maxAngle);
	return 1 - smoothstep(falloutAngle, maxAngle, actualAngle);

	return 1;
}

float CalculateSpotLightFactor(int idx, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[g_LightDataIndex];

	float distFactor = CalculateDistanceFactor(lights[idx].m_Position, worldPosition, lights[idx].m_Distance);
	float angleFactor = CalculateAngleFactor(lights[idx].m_Position, worldPosition, lights[idx].m_Direction, lights[idx].m_Angle);

	return angleFactor * distFactor;
}

float CalculatePointLightFactor(int idx, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[g_LightDataIndex];

	return CalculateDistanceFactor(lights[idx].m_Position, worldPosition, lights[idx].m_Distance);
}

float4 Lighting(float4 albedo, float roughness, float metalic, float ao, float3 worldNormal, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[g_LightDataIndex];

	//float3 viewDir = viewMatrix[2].xyz;
	float3 viewDir = normalize(cameraPosition - worldPosition);
	float4 result = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < g_LightSize; ++i) {
		LIGHT light = lights[i];

		if (light.m_Active == false) continue;

		if (lights[i].m_LightType == 0) {
			result.rgb += 
				//light.m_LightColor.rgb * light.m_Intensity *
				PBRLighting(i, worldNormal, viewDir, -light.m_Direction, albedo, roughness, metalic, ao, worldPosition);
		}		
		else if (lights[i].m_LightType == 1) {
			float3 pointToLight = normalize(light.m_Position - worldPosition);
			result.rgb += 
				//light.m_LightColor.rgb * light.m_Intensity *
				PBRLighting(i, worldNormal, viewDir, pointToLight, albedo, roughness, metalic, ao, worldPosition) * CalculateSpotLightFactor(i, worldPosition);
		}	
		else if (lights[i].m_LightType == 2) {
			float3 pointToLight = normalize(light.m_Position - worldPosition);
			result.rgb += 
				//light.m_LightColor.rgb * light.m_Intensity *
				PBRLighting(i, worldNormal, viewDir, pointToLight, albedo, roughness, metalic, ao, worldPosition) * CalculatePointLightFactor(i, worldPosition);
		}

	}

	return result;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	// sample all
	float4 albedoColor = float4(Tex2DList[g_AlbedoIndex].Sample(samplerWarp, input.uv));
	float4 roughness = float4(Tex2DList[g_RoughnessIndex].Sample(samplerWarp, input.uv));
	float4 metalic = float4(Tex2DList[g_MetallicIndex].Sample(samplerWarp, input.uv));
	float4 ao = float4(Tex2DList[g_AOIndex].Sample(samplerWarp, input.uv));
	float4 normalW = float4(Tex2DList[g_NormalIndex].Sample(samplerWarp, input.uv));
	//normalW *= 2;
	//normalW -= 1;
	float4 positionW = float4(Tex2DList[g_PositionIndex].Sample(samplerWarp, input.uv));
	
	return Lighting(albedoColor, roughness.r, clamp(metalic.r, 0.1f, 1.0f), ao.r, normalize(normalW.rgb), positionW.xyz);
}
