
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
	int albedoIdx;
	int roughIdx;
	int metalIdx;
	int aoIdx;
	int normalIdx;
	int positionIdx;
	int lightSize;
	int lightIdx;
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

float D_GGX(float NdotH, float roughness)
{
	// Trowbridge-Reitz GGX
	float r2 = roughness * roughness;
	float NdH2 = NdotH * NdotH;

	return r2 / (PI * pow((NdH2 * (r2 - 1.0f) + 1.0f), 2.0f));
}

float3 F_Schlick(float3 specColor, float theta)
{
	return specColor + (1.0f - specColor) * pow(1.0f - theta, 5.0f);
}

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

	// faster than /
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

	float3 result = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001f);

	return result;
}

float3 PBRLighting(int idx, float3 normal, float3 viewDir, float3 lightDir, float4 albedo, float roughness, float metalic, float ao, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float shadowFactor = 1;
	if (lights[idx].m_CameraIdx > 0) {
		float dotNormal = dot(normal, -lights[idx].m_Direction);
		shadowFactor = ShadowCalculate(
							float4(worldPosition, 1.0f), 
							dotNormal, 
							lights[idx].m_CameraIdx, 
							lights[idx].m_ShadowMapResults.x);
	}

	return DiffuseBRDF(albedo.rgb, lights[idx].m_LightAmbient.rgb) + SpecularBRDF(albedo.rgb, normal, viewDir, lightDir, roughness, metalic) * shadowFactor;
}

float4 DirectionalLight(int idx, float3 viewDir, float4 albedo, float roughness, float metalic, float3 worldNormal, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	//lights[i];
	// if on light (light map)
	// ambient
	float3 ambient = lights[idx].m_LightAmbient.rgb * 0.2f * albedo.rgb;

	// diffuse
	float3 lightDir = -lights[idx].m_Direction;

	float dotNormal = dot(worldNormal.rgb, lightDir);
	float3 diff = max(dotNormal, float3(0.0f, 0.0f, 0.0f));
	float3 diffuse = lights[idx].m_LightColor.rgb * diff * 0.7f * albedo.rgb;

	float3 refl = reflect(lightDir, worldNormal.rgb);
	float specInt = 0;//pow(max(dot(viewDir, refl), 0.0f), 64.0f);
	float3 specularres = lights[idx].m_LightColor.rgb * specInt * 0.1f * albedo.rgb;

	float shadow = 1;
	if (lights[idx].m_CameraIdx > 0)
		shadow = ShadowCalculate(float4(worldPosition, 1.0f), dotNormal, lights[idx].m_CameraIdx, lights[idx].m_ShadowMapResults.x);

	//shadow = shadow + (1 - worldPosition.w);

	return float4(ambient + shadow * (diffuse + specularres), 0.0f);;
}

float CalculateSpotLightFactor(int idx, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float3 lightToPos = worldPosition.xyz - lights[idx].m_Position;

	float distance = length(lightToPos);
	float distFactor = max(1 - distance / lights[idx].m_Distance, 0);

	float padding = 32.0f;
	float epsilon = 0.2f;
	float theta = dot(normalize(lightToPos), lights[idx].m_Direction);
	float maxCos = cos(lights[idx].m_Angle);
	float spotLightFactor = pow(clamp((theta - maxCos) / epsilon, 0.0f, 1.0f), padding); 

	return spotLightFactor * distFactor;
}

float CalculatePointLightFactor(int idx, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float3 lightToPos = worldPosition - lights[idx].m_Position;
	float distance = length(lightToPos);

	// distance factor
	float factor = max(1 - distance / lights[idx].m_Distance, 0);

	return factor;
}

float4 SpotLight(int idx, float3 viewDir, float4 albedo, float roughness, float metalic, float3 worldNormal, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightToPos = worldPosition.xyz - lights[idx].m_Position;

	float distance = length(lightToPos);
	float distFactor = max(1 - distance / lights[idx].m_Distance, 0);

	float padding = 32.0f;
	float epsilon = 0.2f;
	float theta = dot(normalize(lightToPos), lights[idx].m_Direction);
	float maxCos = cos(lights[idx].m_Angle);
	float spotLightFactor = pow(clamp((theta - maxCos) / epsilon, 0.0f, 1.0f), padding);    

	result = DirectionalLight(idx, viewDir, albedo, roughness, metalic, worldNormal, worldPosition) * spotLightFactor * distFactor;

	return result;
}

float4 PointLight(int idx, float3 viewDir, float4 albedo, float roughness, float metalic, float3 worldNormal, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightToPos = worldPosition - lights[idx].m_Position;
	float distance = length(lightToPos);

	// distance factor
	float factor = max(1 - distance / lights[idx].m_Distance, 0);

	result = DirectionalLight(idx, viewDir, albedo, roughness, metalic, worldNormal, worldPosition) * factor;

	return result;
}

float4 Lighting(float4 albedo, float roughness, float metalic, float ao, float3 worldNormal, float3 worldPosition)
{
	StructuredBuffer<LIGHT> lights = LightDataList[lightIdx];

	//float3 viewDir = viewMatrix[2].xyz;
	float3 viewDir = normalize(cameraPosition - worldPosition);
	float4 result = float4(0.0f, 0.0f, 0.0f, 1.0f);

	//float3 PBRLighting(int idx, float3 normal, float3 viewDir, float3 lightDir, float4 albedo, float roughness, float metalic, float ao, float3 worldPosition)

	for (int i = 0; i < lightSize; ++i) {
		LIGHT light = lights[i];
		//if (light.m_LightType == 0)		result += DirectionalLight(i, viewDir, albedo, roughness, metalic, worldNormal, worldPosition);
		//else if (light.m_LightType == 1)	result += SpotLight(i, viewDir, albedo, roughness, metalic, worldNormal, worldPosition);
		//else if (light.m_LightType == 2)	result += PointLight(i, viewDir, albedo, roughness, metalic, worldNormal, worldPosition);

		if (lights[i].m_LightType == 0) {
			result.rgb += PBRLighting(i, worldNormal, viewDir, -light.m_Direction, albedo, roughness, metalic, ao, worldPosition);
		}		
		else if (lights[i].m_LightType == 1) {
			float3 pointToLight = normalize(light.m_Position - worldPosition);
			result.rgb += PBRLighting(i, worldNormal, viewDir, pointToLight, albedo, roughness, metalic, ao, worldPosition) * CalculateSpotLightFactor(i, worldPosition);
		}	
		else if (lights[i].m_LightType == 2) {
			float3 pointToLight = normalize(light.m_Position - worldPosition);
			result.rgb += PBRLighting(i, worldNormal, viewDir, pointToLight, albedo, roughness, metalic, ao, worldPosition) * CalculatePointLightFactor(i, worldPosition);
		}

	}

	return result;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
	// sample all
	float4 albedoColor = float4(Tex2DList[albedoIdx].Sample(samplerWarp, input.uv));
	float4 roughness = float4(Tex2DList[roughIdx].Sample(samplerWarp, input.uv));
	float4 metalic = float4(Tex2DList[metalIdx].Sample(samplerWarp, input.uv));
	float4 ao = float4(Tex2DList[aoIdx].Sample(samplerWarp, input.uv));
	float4 normalW = float4(Tex2DList[normalIdx].Sample(samplerWarp, input.uv));
	//normalW *= 2;
	//normalW -= 1;
	float4 positionW = float4(Tex2DList[positionIdx].Sample(samplerWarp, input.uv));
	
	return Lighting(albedoColor, roughness.r, metalic.r, ao.r, normalW.rgb, positionW.xyz);

	// do something for post processing
	
	// ambient
	//float3 ambient = lightColor * 0.2f * albedoColor.rgb;
	
	//// diffuse
	//float3 diff = max(dot(normalW.rgb, lightDir), float3(0.0f, 0.0f, 0.0f));
	//float3 diffuse = lightColor * diff * 0.7f * albedoColor.rgb;
	
	//// specular
	//float3 viewDir = viewMatrix[2].xyz;
	//float3 refl = reflect(-lightDir, normalW.rgb);
	//float specInt = pow(max(dot(viewDir, refl), 0.0f), 32.0f);
	//float3 specularres = lightColor * specInt * 0.1f * albedoColor.rgb;
	
	//i.uv = tempUV.yx;
	//return float4(i.tangentW, 1.0f);
	//float3 result = ambient + diffuse + specularres;

	//return float4(result, 1.0f);
}
