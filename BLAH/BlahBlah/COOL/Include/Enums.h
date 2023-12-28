#pragma once


enum ROOT_SIGNATURE_IDX{
	DESCRIPTOR_HEAP = 0,			// 디스크립터힙
	DESCRIPTOR_IDX_CONSTANT,		// 디스크립터를 사용할 인덱스 16개의 int
	CAMERA_DATA_CBV,				// 카메라행렬 4x4 x2
	WORLD_MATRIX,
	SHADER_DATAS_CBV,				// 뭐 대충 나머지들 (delta time, 등등)
	ROOT_SIGNATURE_IDX_MAX
};

enum MATERIAL_TYPES {
	BaseColor = 0,
	Roughness,
	Metalic,
	Specular,
	Normal,
	MATERIAL_END
};
