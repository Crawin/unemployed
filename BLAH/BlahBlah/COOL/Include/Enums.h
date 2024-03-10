#pragma once


enum class ROOT_SIGNATURE_IDX{
	DESCRIPTOR_HEAP = 0,			// 디스크립터힙
	DESCRIPTOR_IDX_CONSTANT,		// 디스크립터를 사용할 인덱스 16개의 int
	CAMERA_DATA_CBV,				// 카메라행렬 4x4 x2
	WORLD_MATRIX,
	SHADER_DATAS_CBV,				// 뭐 대충 나머지들 (delta time, 등등)
	ROOT_SIGNATURE_IDX_MAX
};

enum class MATERIAL_TYPES {
	BaseColor = 0,
	Roughness,
	Metalic,
	Specular,
	Normal,
	MATERIAL_END
};

// struct
//struct PS_MRT_OUTPUT
//{
//	float4 Albedo : SV_TARGET0;
//	float4 Roughness : SV_TARGET1;
//	float4 Metalic : SV_TARGET2;
//	float4 Specular : SV_TARGET3;
//	float4 NormalW : SV_TARGET4;
//	float4 PositionW : SV_Target5;
//};

// PBR
// MATERIAL_TYPES랑 같이 쓸까?
// ㄴㄴ
enum class MULTIPLE_RENDER_TARGETS {
	BaseColor = 0,
	Roughness,
	Metalic,
	Specular,
	NormalW,
	PositionW,
	MRT_END
};
